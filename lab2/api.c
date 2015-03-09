#include "api.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif

#define MASTER      0           // Master process ID
#define WORK_TAG    1           // This tag indicates a message contains a work load
#define DONE_TAG    2           // This is the master telling the worker it can stop
#define RESULT_TAG  3           // Indicates that this message contains a finished piece of work

/* **** TODO **** 
  - Change round robbin technique (optional)
  - Check report_results value to see if it succeeded or not
  - Check the create_work function to see if it suceeded or not 
  - Check if malloc fails
  */

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
      int work_chunk_count = 0;
      while(work_chunks[i] != NULL) {

        work_chunk_count++;
        // get the next work chunk
        one_work_t *work_chunk = work_chunks[i++];

        // send the chunk to a processor, roundrobbin style
        debug_print("Process %d out of %d\n", process_num, sz);
        MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process_num,
          WORK_TAG, MPI_COMM_WORLD);

        if (++process_num >= sz) { // make sure we're roundrobbining.
          process_num = 1;
        }

        debug_print("Nothing went wrong in iter %d\n", i);
      }
      //This was allocated in the user's do_work function
      for(i = 0; i < work_chunk_count; ++i)
      {
          free(work_chunks[i]);
      }
      free(work_chunks);

      //We receive results..
      one_result_t **result_array = (one_result_t**)malloc(sizeof(one_result_t*)*work_chunk_count);
      for(i = 0; i < work_chunk_count; ++i)
      {
          one_result_t *result = (one_result_t*)malloc((f->result_sz));
          MPI_Status status;

          MPI_Recv(result, f->result_sz, MPI_CHAR, MPI_ANY_SOURCE, RESULT_TAG, 
            MPI_COMM_WORLD, &status);

          result_array[i] = result;
      }

      //Tell workers to stop running
      for (i = 1; i < sz; ++i)
      {
          int num = 1;
          MPI_Send(&num, 1, MPI_INT, i, DONE_TAG, MPI_COMM_WORLD);
      }

      //Report result!
      f->report_results(work_chunk_count, result_array);

      // Free memeory of results array
      for(i = 0; i < work_chunk_count; ++i)
      {
          free(result_array[i]);
      }
      free(result_array);
    }

    // workers do work
    else
    {
      debug_print("Hola, desde processor %d\n", myid);
      int i = 1;
      while(i > 0) {
          one_work_t *work_chunk = (one_work_t *)malloc(f->work_sz);
          if (!work_chunk) {
              printf("Failed to allocate space for a work chunk on a worker\n");
              exit(1);
          }

          MPI_Status status;
          MPI_Recv(work_chunk, f->work_sz, MPI_CHAR, MASTER, MPI_ANY_TAG, 
            MPI_COMM_WORLD, &status);

          if (status.MPI_TAG == WORK_TAG) 
          {
              one_result_t *result = f->do_one_work(work_chunk);
              MPI_Send(result, f->result_sz, MPI_CHAR, MASTER, 
                RESULT_TAG, MPI_COMM_WORLD);
              free(result);
          } 
          else if(status.MPI_TAG == DONE_TAG)
          {
              i = -1;
              debug_print("Worker %d says: He terminado!\n", myid);
          }
          else
          {
             printf("Unexpected Error... Process %d received message with unknown tag.\n", myid);
          }
          free(work_chunk);

      }
    }
    debug_print("Finished Running on processor %d\n", myid);

}




