#!/bin/bash

# Exit on error
set -e

# ============================================
# CONFIGURATION - Adjust as needed
# ============================================
# Maximum number of input files to process
# Set to -1 to process all files, or a positive number to limit
MAX_FILES=10  # Example: 5 will process input1.csv to input5.csv

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
    
    # Extract lns_perc from config file
    LNS_PERC=$(grep "lns_perc" "$CONFIG_FILE" | awk '{print $2}')
    if [ -z "$LNS_PERC" ]; then
        echo "Warning: Could not find lns_perc in config file. Using default 0.2"
        LNS_PERC="0.2"
    fi
    
    # Define output files with config name
    RESULTS_FILE="lns_statistics_results_${config_name}.csv"
    SUMMARY_FILE="lns_statistics_summary_${config_name}.txt"
    
    echo "========================================"
    echo "Processing Config: $config_name"
    echo "LNS Percentage: $LNS_PERC"
    echo "Output CSV: $RESULTS_FILE"
    echo "========================================"
    echo ""
    
    # Initialize CSV header with clean output
    echo "Input File,Classes,Professors,LNS Perc,Total Nodes,Nodes Left,Total Time,Memory Used,LNS Time,LNS Sols Found,Best Sol At Node,Best Sol Found By" > "$RESULTS_FILE"
    
    # Initialize counters
    total_files=0
    successful_runs=0
    failed_runs=0
    total_time=0
    total_nodes=0
    lns_found_count=0
    
    # Arrays for statistics
    declare -a execution_times=()
    declare -a node_counts=()
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
        if xargs -a "$CONFIG_FILE" ./bin/dpd "$input_file" > /dev/null 2>"$error_output"; then
            echo "    ✓ Execution successful"
            successful_runs=$((successful_runs + 1))
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
                
                # Optional fields (best solution info)
                best_sol_node="${FIELDS[14]:-N/A}"
                best_sol_time="${FIELDS[15]:-N/A}"
                best_sol_depth="${FIELDS[16]:-N/A}"
                best_sol_heur="${FIELDS[17]:-N/A}"
                
                # LNS heuristic info (if available)
                lns_time="${FIELDS[18]:-0}"
                lns_calls="${FIELDS[19]:-N/A}"
                lns_sols="${FIELDS[20]:-0}"
                lns_best="${FIELDS[21]:-N/A}"
                heur_name="${FIELDS[22]:-N/A}"
                config="${FIELDS[23]:-N/A}"
                
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
                
                # Check if LNS found a solution
                if [ "$heur_name" = "lns" ] || echo "$line" | grep -q "lns"; then
                    lns_found_count=$((lns_found_count + 1))
                    lns_success+=("$filename")
                    echo "    ✓ LNS found solution!"
                fi
                
                # Accumulate statistics
                total_time=$(awk "BEGIN {print $total_time + $total_time_val}")
                total_nodes=$((total_nodes + total_nodes_val))
                execution_times+=("$total_time_val")
                node_counts+=("$total_nodes_val")
                
                # Write to CSV - Clean output with only essential fields
                echo "$filename,$num_classes,$num_professors,$LNS_PERC,$total_nodes_val,$nodes_left,$total_time_val,$memory,$lns_time,$lns_sols,$best_sol_node_num,$best_sol_finder" >> "$RESULTS_FILE"
                
                echo "    Time: ${total_time_val}s, Nodes: $total_nodes_val, Best Sol: Node $best_sol_node_num by '$best_sol_finder'"
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
    if [ ${#execution_times[@]} -gt 0 ]; then
        avg_time=$(awk 'BEGIN {sum=0; for(i=0; i<'${#execution_times[@]}'; i++) sum+='${execution_times[i]}'; print sum/'${#execution_times[@]}'}')
        avg_nodes=$(awk 'BEGIN {sum=0; for(i=0; i<'${#node_counts[@]}'; i++) sum+='${node_counts[i]}'; print sum/'${#node_counts[@]}'}')
    else
        avg_time=0
        avg_nodes=0
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
        echo "  LNS Percentage: $LNS_PERC"
        echo "  Config File: $CONFIG_FILE"
        echo "  Input files processed: ${#files_to_process[@]}"
        echo ""
        echo "EXECUTION SUMMARY:"
        echo "  Total files processed: $total_files"
        echo "  Successful runs: $successful_runs"
        echo "  Failed runs: $failed_runs"
        echo "  Success rate: $(awk "BEGIN {printf \"%.2f\", ($successful_runs/$total_files)*100}")%"
        echo ""
        echo "LNS HEURISTIC PERFORMANCE:"
        echo "  Solutions found by LNS: $lns_found_count"
        echo "  LNS success rate: $(awk "BEGIN {printf \"%.2f\", ($lns_found_count/$total_files)*100}")%"
        echo ""
        echo "TIME STATISTICS:"
        echo "  Total execution time: $(printf "%.2f" $total_time)s"
        echo "  Average time per instance: $(printf "%.2f" $avg_time)s"
        echo ""
        echo "NODE STATISTICS:"
        echo "  Total nodes explored: $total_nodes"
        echo "  Average nodes per instance: $(printf "%.0f" $avg_nodes)"
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
