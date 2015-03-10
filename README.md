# ParallelAndDistributed
Labs for CS5460 -- Parallel and Distributed Systems -- Spring 2015

To compile and run test.c with 4 processors:

mpicc api.c test.c -o test

mpirun -np 4 test
