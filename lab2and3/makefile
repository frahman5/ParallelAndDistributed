compile: factor.c api.c
	mpicc -lgmp api.c factor.c -o factor_run
run:
	mpirun -np $(p) factor_run $(f)
