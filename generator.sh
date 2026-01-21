#!/bin/bash

# Exit on error
set -e

# Check for the number of iterations as a parameter
if [ -z "$1" ]; then
    echo "Usage: $0 <number_of_iterations>"
    exit 1
fi

N=$1
infinite_count=0
lns_solution_count=0

for (( run=1; run<=N; run++ )); do
    echo "--- Iteration ${run}/${N} ---"

    # Run the Python script to generate the input file
    echo "Generating input file with 'python3 main.py x y'..."
    python3 main.py 25 2

    # Run the DPD solver and capture its output to a variable and also display it
    echo "Running DPD solver..."
    output=$(xargs -a ./configs/teste.config ./bin/dpd saida.csv | tee /dev/tty)

    # Display the solver's output
    echo "Solver finished. Analyzing output."

    # Check for "infinite" in the last 20 lines
    if echo "$output" | tail -n 20 | grep -q "infinite"; then
        echo "Found 'infinite' in the last 20 lines of output."
        infinite_count=$((infinite_count + 1))
    fi

    # Check if the last 20 lines of the output contain "found by <lns>"
    if echo "$output" | tail -n 20 | grep -q "found by <lns>"; then
        echo "Heuristic 'lns' found a solution."
        lns_solution_count=$((lns_solution_count + 1))

        # Find the highest existing file number in input/lns/
        last_num=0
        for f in input/lns/input*.csv; do
            # Check if any file matches the pattern to avoid errors when the directory is empty
            if [[ -f "$f" ]]; then
                # Extract number from filename (e.g., input12.csv -> 12)
                num=$(basename "$f" .csv | sed 's/input//')
                if [[ "$num" -gt "$last_num" ]]; then
                    last_num=$num
                fi
            fi
        done
        
        i=$((last_num + 1))

        new_filename="input/lns/input${i}.csv"
        echo "Moving 'saida.csv' to '$new_filename'"
        mv saida.csv "$new_filename"
    else
        echo "Condition not met. 'saida.csv' will not be moved."
    fi
done

echo "--- All $N iterations complete. ---"
echo "Total times 'infinite' was found: $infinite_count"
echo "Total times 'Heuristic 'lns' found a solution.': $lns_solution_count"
