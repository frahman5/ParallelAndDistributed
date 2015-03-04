#include "api.h"
#include <mpi.h>
#include <stdio.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif

#define MASTER      0           // Master process ID
#define WORK_TAG    1           // This tag indicates a message contains a work load
#define DONE_TAG    2           // This is the master telling the worker it can stop
#define RESULT_TAG  3           // Indicates that this message contains a finished piece of work


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
      // Create all the work to be done
    	one_work_t **work_chunks = f->create_work_pool(argc, argv);

      // Send out the work to workers round-robbin style
      int i = 0;
      int process_num = 1;
      while(work_chunks[i] != NULL) {
        // get the next work chunk
        one_work_t *work_chunk = work_chunks[i++];

        // send the chunk to a processor, roundrobbin style
        MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process_num,
          WORK_TAG, MPI_COMM_WORLD);
        if (++process_num > sz) { // make sure we're roundrobbining.
          process_num = 1;
        }
        debug_print("Nothing went wrong in iter %d\n", i);
      }

      // TODO!!!!: Receive the results from workers, round robbin style
    }

    // workers do work
    else
    {
      int i = 1;
      while(i > 0) {
          one_work_t *work_chunk = (one_work_t *)malloc(f->work_sz);
          MPI_Status status;
          MPI_Recv(work_chunk, f->work_sz, MPI_CHAR, MASTER, *, 
            MPI_COMM_WORLD, &status);

          if (status.MPI_TAG == WORK_TAG) {
              one_result_t *result = f->do_one_work(work_chunk);
              MPI_Send(result, f->result_sz, MPI_CHAR, MASTER, 
                RESULT_TAG, MPI_COMM_WORLD);
          } else {
              i = -1;
              printf("Worker %d says: He terminado!\n", myid);
          }
      }
    }

    debug_print("Finished Running\n");

}




