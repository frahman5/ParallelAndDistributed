#!/bin/bash
# Remove the old version and compile a new one
rm factor_run
mpicc api.c factor.c -lgmp -o factor_run

# Run the tests
for i in `seq 2 3`;
    do
        echo "Running with $i processors on multiple: 2626088838234521" >> $1
        time mpirun -np $i -hostfile hosts factor_run 2626088838234521  >> $1 2>&1
    done
