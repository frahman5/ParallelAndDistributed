#include "api.h"
#include <mpi.h>
#include <stdio.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif

#define MASTER      0           // Master process ID


/****

	This function does the following:
		- Receives api object 
		- Uses the create_work_pool function to get the array of workers
		- Goes through the array and executes the necessary work for each
		- Prints the results
	
****/
void MW_Run (int argc, char **argv, struct mw_fxns *f){
	
	// Initialization of parameters
    int sz, myid;
    MPI_Comm_size (MPI_COMM_WORLD, &sz);
  	MPI_Comm_rank (MPI_COMM_WORLD, &myid); 

  	// master process sets up calculation
  	if(myid == MASTER)
    {
    	one_work_t ** work_chunks = f->create_work_pool(argc, argv);
    	int i = 0;
    	for(i = 0; i < f->chunks_num; ++i)
    	{
    		one_work_t * work_chunk = work_chunks[i];
    		debug_print("%llu",work_chunk->sz);
    	}

    }
    else
    {

    }

    debug_print("Finished Running\n");

}




