#!/bin/bash

# Exit on error
set -e

# ============================================
# CONFIGURATION - Adjust as needed
# ============================================
# Maximum number of input files to process
# Set MAX_FILES in the environment to limit the run, or use -1 for all files
MAX_FILES=3

# Pre-define specific files to process (optional)
# If set, these files will be used instead of searching/selecting randomly
# Leave empty to use automatic file selection
# Example: FILES_TO_PROCESS=("input/lns/input1.csv" "input/lns/input5.csv" "input/lns/input10.csv")
FILES_TO_PROCESS=()
# ============================================

# Define directories and files
INPUT_DIR="input/lns"
OUTPUT_DIR="output"

# Check if at least one config file is provided
if [ $# -eq 0 ]; then
    echo "Error: No config files provided."
    echo "Usage: $0 <config_file1> [config_file2] [config_file3] ..."
    echo "Example: $0 ./configs/teste_lns_25.config ./configs/teste_lns_50.config ./configs/teste_lns_75.config"
    exit 1
fi

# Store all config files
CONFIG_FILES=("$@")

# Check if input directory exists
if [ ! -d "$INPUT_DIR" ]; then
    echo "Error: Directory '$INPUT_DIR' does not exist."
    exit 1
fi

# Check if all config files exist
for config in "${CONFIG_FILES[@]}"; do
    if [ ! -f "$config" ]; then
        echo "Error: Config file '$config' does not exist."
        exit 1
    fi
done

echo "========================================="
echo "LNS Statistics Collection Runner"
echo "Started at: $(date)"
echo "Config files to process: ${#CONFIG_FILES[@]}"
for cfg in "${CONFIG_FILES[@]}"; do
    echo "  - $(basename "$cfg")"
done
if [ ${#FILES_TO_PROCESS[@]} -gt 0 ]; then
    echo "Input files: ${#FILES_TO_PROCESS[@]} (pre-defined)"
elif [ "$MAX_FILES" -eq -1 ]; then
    echo "Input files: ALL available"
else
    echo "Input files: $MAX_FILES (randomly selected)"
fi
echo "========================================="
echo ""

# Check if files are pre-defined
if [ ${#FILES_TO_PROCESS[@]} -gt 0 ]; then
    echo "Using pre-defined input files..."
    files_to_process=("${FILES_TO_PROCESS[@]}")
    echo "Total input files to process: ${#files_to_process[@]}"
    for file in "${files_to_process[@]}"; do
        if [ ! -f "$file" ]; then
            echo "Warning: Pre-defined file not found: $file"
        fi
    done
else
    # Collect all available input files ONCE
    echo "Collecting input files..."
    all_input_files=()
    for input_file in "$INPUT_DIR"/input*.csv; do
        if [ -f "$input_file" ]; then
            all_input_files+=("$input_file")
        fi
    done

    # Randomly select files if MAX_FILES is set
    if [ "$MAX_FILES" -ne -1 ] && [ "$MAX_FILES" -gt 0 ]; then
        total_available=${#all_input_files[@]}
        if [ "$MAX_FILES" -ge "$total_available" ]; then
            echo "Requested $MAX_FILES files, but only $total_available available. Using all files."
            files_to_process=("${all_input_files[@]}")
        else
            # Randomly shuffle and select MAX_FILES files
            IFS=$'\n' files_to_process=($(printf '%s\n' "${all_input_files[@]}" | shuf -n "$MAX_FILES"))
            unset IFS
            echo "Randomly selected $MAX_FILES files from $total_available available files."
        fi
    else
        echo "Processing ALL available files."
        files_to_process=("${all_input_files[@]}")
    fi

    # Sort the selected files for consistent output
    IFS=$'\n' files_to_process=($(printf '%s\n' "${files_to_process[@]}" | sort -V))
    unset IFS

    echo "Total input files to process: ${#files_to_process[@]}"
fi
echo ""

# Loop through each config file
for CONFIG_FILE in "${CONFIG_FILES[@]}"; do
    config_name=$(basename "$CONFIG_FILE" .config)

    LNS_ENABLED=$(awk '$1 == "--heur_lns" { value = $2 } END { print value + 0 }' "$CONFIG_FILE")
    GRASP_ENABLED=$(awk '$1 == "--heur_grasp" { value = $2 } END { print value + 0 }' "$CONFIG_FILE")
    BAD_SOL_ENABLED=$(awk '$1 == "--heur_bad_sol" { value = $2 } END { print value + 0 }' "$CONFIG_FILE")

    CONFIG_ARGUMENTS=()
    while IFS= read -r config_line || [ -n "$config_line" ]; do
        if [ -z "$config_line" ] || [[ "$config_line" =~ ^[[:space:]]*# ]]; then
            continue
        fi
        read -ra config_line_arguments <<< "$config_line"
        CONFIG_ARGUMENTS+=("${config_line_arguments[@]}")
    done < "$CONFIG_FILE"
    
    # Extract lns_perc from config file
    LNS_PERC=$(awk '$1 == "--lns_perc" { print $2; found = 1 } END { if (!found) print "" }' "$CONFIG_FILE")
    if [ -z "$LNS_PERC" ] && [ "$LNS_ENABLED" -eq 1 ]; then
        echo "Warning: Could not find lns_perc in config file. Using default 0.2"
        LNS_PERC="0.2"
    fi
    if [ "$LNS_ENABLED" -eq 0 ]; then
        LNS_PERC="0"
    fi
    
    # Define output files with config name
    RESULTS_FILE="lns_statistics_results_${config_name}.csv"
    SUMMARY_FILE="lns_statistics_summary_${config_name}.txt"
    
    echo "========================================"
    echo "Processing Config: $config_name"
    echo "LNS Enabled: $LNS_ENABLED"
    echo "GRASP Enabled: $GRASP_ENABLED"
    echo "LNS Percentage: $LNS_PERC"
    echo "Output CSV: $RESULTS_FILE"
    echo "========================================"
    echo ""
    
    # Initialize CSV header with clean output
    echo "Input File,Classes,Professors,LNS Perc,Total Nodes,Nodes Left,Total Time,Gap,Memory Used,LNS Time,LNS Calls,LNS Executions,LNS Sols Found,LNS Best Sols Found,GRASP Time,GRASP Calls,GRASP Sols Found,GRASP Best Sols Found,Best Sol At Node,Best Sol Found By" > "$RESULTS_FILE"
    
    # Initialize counters
    total_files=0
    successful_runs=0
    failed_runs=0
    total_time=0
    total_nodes=0
    total_nodes_left=0
    total_gap=0
    total_lns_calls=0
    total_lns_executions=0
    total_lns_sols=0
    lns_found_count=0
    
    # Arrays for statistics
    declare -a lns_success=()
    
    # Loop through selected input files
    for input_file in "${files_to_process[@]}"; do
        # Check if file exists
        if [ ! -f "$input_file" ]; then
            continue
        fi

        total_files=$((total_files + 1))
        filename=$(basename "$input_file")
        
        echo "  [$total_files/${#files_to_process[@]}] Processing: $filename"
        
        # Extract number of classes and professors from the first line of the input file
        first_line=$(head -n 1 "$input_file")
        IFS=';' read -ra HEADER <<< "$first_line"
        num_classes="${HEADER[0]}"
        num_professors="${HEADER[1]}"
        
        echo "    Classes: $num_classes, Professors: $num_professors"
        
        # Run the DPD solver - capture error output
        error_output=$(mktemp)
        if ./bin/dpd "$input_file" "${CONFIG_ARGUMENTS[@]}" > /dev/null 2>"$error_output"; then
            echo "    ✓ Execution successful"
            rm -f "$error_output"
            
            # Find the most recent output directory
            latest_output=$(ls -td "$OUTPUT_DIR"/*/ | head -1)
            
            if [ -z "$latest_output" ]; then
                echo "    ✗ Error: Could not find output directory"
                failed_runs=$((failed_runs + 1))
                continue
            fi
            
            # Construct the output file name
            output_file="${latest_output}${filename}-dpd-${config_name}.config.out"
            
            if [ ! -f "$output_file" ]; then
                echo "    ✗ Error: Output file not found: $output_file"
                failed_runs=$((failed_runs + 1))
                continue
            fi
            
            # Read the output file and parse the data
            if [ -f "$output_file" ]; then
                # Read the entire line (should be one line with semicolon-separated values)
                line=$(cat "$output_file")
                
                # Split by semicolon and extract values
                IFS=';' read -ra FIELDS <<< "$line"
                
                # Extract key fields
                root_iters="${FIELDS[1]}"
                exec_time="${FIELDS[2]}"
                primal_bound="${FIELDS[3]}"
                dual_bound="${FIELDS[4]}"
                gap="${FIELDS[5]}"
                dual_root="${FIELDS[6]}"
                total_nodes_val="${FIELDS[7]}"
                nodes_left="${FIELDS[8]}"
                solving_time="${FIELDS[9]}"
                total_time_val="${FIELDS[10]}"
                memory="${FIELDS[11]}"
                lp_cols="${FIELDS[12]}"
                status="${FIELDS[13]}"
                
                best_sol_node="N/A"
                best_sol_time="N/A"
                best_sol_depth="N/A"
                best_sol_heur="N/A"
                stats_index=14

                if [[ "${FIELDS[14]:-}" =~ ^bestsol\ in\ ([0-9]+)$ ]]; then
                    best_sol_node="${FIELDS[14]}"
                    best_sol_time="${FIELDS[15]:-N/A}"
                    best_sol_depth="${FIELDS[16]:-N/A}"
                    best_sol_heur="${FIELDS[17]:-N/A}"
                    stats_index=18
                fi

                lns_time=0
                lns_calls=0
                lns_executions=0
                lns_sols=0
                lns_best=0
                lns_name="disabled"

                if [ "$LNS_ENABLED" -eq 1 ]; then
                    lns_time="${FIELDS[$stats_index]:-}"
                    lns_calls="${FIELDS[$((stats_index + 1))]:-}"
                    lns_executions="${FIELDS[$((stats_index + 2))]:-}"
                    lns_sols="${FIELDS[$((stats_index + 3))]:-}"
                    lns_best="${FIELDS[$((stats_index + 4))]:-}"
                    lns_name="${FIELDS[$((stats_index + 5))]:-}"
                    stats_index=$((stats_index + 6))

                    if [ "$lns_name" != "lns" ]; then
                        echo "    ✗ Error: Invalid LNS statistics block in $output_file"
                        failed_runs=$((failed_runs + 1))
                        continue
                    fi
                fi

                if [ "$BAD_SOL_ENABLED" -eq 1 ]; then
                    bad_sol_name="${FIELDS[$((stats_index + 4))]:-}"
                    stats_index=$((stats_index + 5))

                    if [ "$bad_sol_name" != "badFeasibleSolution" ]; then
                        echo "    ✗ Error: Invalid bad-solution statistics block in $output_file"
                        failed_runs=$((failed_runs + 1))
                        continue
                    fi
                fi

                grasp_time=0
                grasp_calls=0
                grasp_sols=0
                grasp_best=0
                grasp_name="disabled"

                if [ "$GRASP_ENABLED" -eq 1 ]; then
                    grasp_time="${FIELDS[$stats_index]:-}"
                    grasp_calls="${FIELDS[$((stats_index + 1))]:-}"
                    grasp_sols="${FIELDS[$((stats_index + 2))]:-}"
                    grasp_best="${FIELDS[$((stats_index + 3))]:-}"
                    grasp_name="${FIELDS[$((stats_index + 4))]:-}"
                    stats_index=$((stats_index + 5))

                    if [ "$grasp_name" != "grasp" ]; then
                        echo "    ✗ Error: Invalid GRASP statistics block in $output_file"
                        failed_runs=$((failed_runs + 1))
                        continue
                    fi
                fi

                config="${FIELDS[$stats_index]:-}"
                if [ "$config" != "${config_name}.config" ]; then
                    echo "    ✗ Error: Expected config '${config_name}.config', found '$config' in $output_file"
                    failed_runs=$((failed_runs + 1))
                    continue
                fi
                
                # Extract best sol node number from "bestsol in X" format
                if [[ "$best_sol_node" =~ bestsol\ in\ ([0-9]+) ]]; then
                    best_sol_node_num="${BASH_REMATCH[1]}"
                else
                    best_sol_node_num="N/A"
                fi
                
                # Clean up the best_sol_heur field (who found the best solution)
                if [ "$best_sol_heur" = "N/A" ] || [ -z "$best_sol_heur" ]; then
                    best_sol_finder="unknown"
                else
                    best_sol_finder="$best_sol_heur"
                fi
                
                # Check if LNS found at least one solution
                if [[ "$lns_sols" =~ ^[0-9]+$ ]] && [ "$lns_sols" -gt 0 ]; then
                    lns_found_count=$((lns_found_count + 1))
                    lns_success+=("$filename")
                    echo "    ✓ LNS found solution!"
                fi
                
                # Accumulate statistics
                total_time=$(awk -v total="$total_time" -v value="$total_time_val" 'BEGIN { print total + value }')
                total_nodes=$((total_nodes + total_nodes_val))
                total_nodes_left=$((total_nodes_left + nodes_left))
                total_gap=$(awk -v total="$total_gap" -v value="$gap" 'BEGIN { print total + value }')
                total_lns_calls=$((total_lns_calls + lns_calls))
                total_lns_executions=$((total_lns_executions + lns_executions))
                total_lns_sols=$((total_lns_sols + lns_sols))
                successful_runs=$((successful_runs + 1))
                
                # Write to CSV - Clean output with only essential fields
                echo "$filename,$num_classes,$num_professors,$LNS_PERC,$total_nodes_val,$nodes_left,$total_time_val,$gap,$memory,$lns_time,$lns_calls,$lns_executions,$lns_sols,$lns_best,$grasp_time,$grasp_calls,$grasp_sols,$grasp_best,$best_sol_node_num,$best_sol_finder" >> "$RESULTS_FILE"
                
                echo "    Time: ${total_time_val}s, Gap: $gap, Nodes: $total_nodes_val, Nodes left: $nodes_left, Best Sol: Node $best_sol_node_num by '$best_sol_finder'"
            fi
        else
            echo "    ✗ Execution failed"
            # Show error details
            if [ -f "$error_output" ]; then
                echo "    Error details:"
                head -20 "$error_output" | sed 's/^/      /'
                rm -f "$error_output"
            fi
            failed_runs=$((failed_runs + 1))
        fi
        
        echo ""
    done
    
    # Calculate statistics
    if [ "$successful_runs" -gt 0 ]; then
        avg_time=$(awk -v total="$total_time" -v count="$successful_runs" 'BEGIN { print total / count }')
        avg_nodes=$(awk -v total="$total_nodes" -v count="$successful_runs" 'BEGIN { print total / count }')
        avg_nodes_left=$(awk -v total="$total_nodes_left" -v count="$successful_runs" 'BEGIN { print total / count }')
        success_rate=$(awk -v successful="$successful_runs" -v total="$total_files" 'BEGIN { printf "%.2f", (successful / total) * 100 }')
        lns_success_rate=$(awk -v found="$lns_found_count" -v total="$successful_runs" 'BEGIN { printf "%.2f", (found / total) * 100 }')
    else
        avg_time=0
        avg_nodes=0
        avg_nodes_left=0
        success_rate=0
        lns_success_rate=0
    fi
    
    # Generate summary report
    {
        echo "========================================="
        echo "LNS STATISTICS SUMMARY REPORT"
        echo "Config: $config_name"
        echo "Completed at: $(date)"
        echo "========================================="
        echo ""
        echo "CONFIGURATION:"
        echo "  LNS enabled: $LNS_ENABLED"
        echo "  GRASP enabled: $GRASP_ENABLED"
        echo "  LNS Percentage: $LNS_PERC"
        echo "  Config File: $CONFIG_FILE"
        echo "  Input files processed: ${#files_to_process[@]}"
        echo ""
        echo "EXECUTION SUMMARY:"
        echo "  Total files processed: $total_files"
        echo "  Successful runs: $successful_runs"
        echo "  Failed runs: $failed_runs"
        echo "  Success rate: ${success_rate}%"
        echo ""
        echo "LNS HEURISTIC PERFORMANCE:"
        echo "  SCIP callback calls: $total_lns_calls"
        echo "  Effective LNS executions: $total_lns_executions"
        echo "  Solutions produced by LNS: $total_lns_sols"
        echo "  Solutions found by LNS: $lns_found_count"
        echo "  LNS success rate: ${lns_success_rate}%"
        echo ""
        echo "TIME STATISTICS:"
        echo "  Total execution time: $(printf "%.2f" $total_time)s"
        echo "  Average time per instance: $(printf "%.2f" $avg_time)s"
        echo ""
        echo "NODE STATISTICS:"
        echo "  Total nodes explored: $total_nodes"
        echo "  Average nodes per instance: $(printf "%.0f" $avg_nodes)"
        echo "  Total nodes left: $total_nodes_left"
        echo "  Average nodes left per instance: $(printf "%.0f" $avg_nodes_left)"
        echo ""
        echo "GAP STATISTICS:"
        echo "  Sum of gaps: $(printf "%.6f" $total_gap)"
        echo ""
        
        if [ ${#lns_success[@]} -gt 0 ]; then
            echo "FILES WHERE LNS FOUND SOLUTIONS:"
            for file in "${lns_success[@]}"; do
                echo "  - $file"
            done
            echo ""
        fi
        
        echo "DETAILED RESULTS:"
        echo "  CSV file: $RESULTS_FILE"
        echo ""
        echo "========================================="
    } | tee "$SUMMARY_FILE"
    
    echo ""
    echo "✓ Config '$config_name' complete!"
    echo "✓ Detailed results saved to: $RESULTS_FILE"
    echo "✓ Summary saved to: $SUMMARY_FILE"
    echo ""
    echo "========================================="
    echo ""
done

echo ""
echo "========================================="
echo "ALL CONFIGURATIONS COMPLETED!"
echo "Finished at: $(date)"
echo "========================================="
