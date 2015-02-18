#!/bin/bash
        for i in `seq 1 8`;
        do
		for j in 'seq 1 5';
		do
			echo $i
                	mpirun -np $i -hostfile hosts flops
        	done
	done
