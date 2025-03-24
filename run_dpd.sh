#!/bin/bash

# Loop from 1 to 20
for ((x=1; x<=20; x++)); do
    # Run the command with the current value of x
    ./bin/dpd instancias/saida_$x.csv --time 900
done
