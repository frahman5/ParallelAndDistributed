#!/bin/bash
for i in `seq 2 16`;
    do
        echo "Running with $i processors on multiple: 2626088838234521"
        time mpirun -np $i -hostfile hosts factor_run 2626088838234521
    done
