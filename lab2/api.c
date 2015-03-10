#include "api.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
  - Check the create_work function to see if it suceeded or not 
  */

/****

	This function does the following:
		- Receives api object 
		- Uses the create_work_pool function to get the array of workers
		- Goes through the array and executes the necessary work for each
		- Prints the results
	
****/

/*==============================================================*/
/* MW_Run with Dynamic Process selection                        */
/*==============================================================*/
void MW_Run_2 (int argc, char **argv, struct mw_fxns *f){
  
    // Counts number of messages sent (for granularity)
    unsigned long num_msgs = 0;

    // Initialization of MPI parameters
    int sz, myid;
    MPI_Comm_size (MPI_COMM_WORLD, &sz);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid); 

    // master process sets up calculation
    if(myid == MASTER)
    {
        /* Create all the work to be done and find out how much work we have total */
        one_work_t **work_chunks = f->create_work_pool(argc, argv);
        int total_num_chunks = 0;
        while(work_chunks[total_num_chunks] != NULL) {
            total_num_chunks += 1;
        }
        printf("How many work chunks?: %d\n", total_num_chunks);

        /* Send out the first batch of work to workers round-robbin style */
        int process_num;
        int current_work_chunk = 0;
        for (process_num = 1; process_num < sz; process_num++) {

            // Send the next work chunk
            one_work_t *work_chunk = work_chunks[current_work_chunk];
            if (work_chunk) {
                printf("Sending Process %d out of %d their first work chunk. Work chunk count: %d\n", process_num, sz, current_work_chunk);
                MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process_num,
                    WORK_TAG, MPI_COMM_WORLD);
                current_work_chunk++;
                num_msgs++;
            } else {
                process_num = sz;                           // exit the for loop
            }
        }
        printf("Work chunks sent out on initial round robbin pass: %d\n", current_work_chunk);


        /* If necessary, send out remaining work-chunks dynamically
            When a worker says they are done, send them new work! */
        int num_results_received = 0;
        one_result_t **result_array = (one_result_t **)malloc(sizeof(one_result_t*) * total_num_chunks);
        if (!result_array) {
            printf("MW library failed to allocate result_array on the heap\n");
            exit(1);
        }
        while (num_results_received != total_num_chunks) {

            // Create a container for the result
            printf("getting result %d of %d\n", num_results_received + 1, total_num_chunks);
            one_result_t *result = (one_result_t *)malloc((f->result_sz));
            if (!result) {
              printf("MW Library failed to allocate a result struct on the heap\n");
              exit(1);
            }

            // Receive the result and store it
            MPI_Status status;
            MPI_Recv(result, f->result_sz, MPI_CHAR, MPI_ANY_SOURCE, RESULT_TAG, 
                MPI_COMM_WORLD, &status);
            result_array[num_results_received++] = result;        
            num_msgs++;                             // "global" message counter

            // If there's more work to be done, send the worker who just finished more work
            if (current_work_chunk < total_num_chunks) {

                // Which process do we send it to?
                int process = status.MPI_SOURCE;

                // Extract the next work chunk
                one_work_t *work_chunk = work_chunks[current_work_chunk++];
                assert (work_chunk != NULL);

                // Send it and increment relevant counters
                MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process,
                    WORK_TAG, MPI_COMM_WORLD);
                num_msgs++;
            }
        }
        assert (current_work_chunk == total_num_chunks);

        /* Free up work chunks: This was allocated in the user's do_work function */
        int i = 0;
        while(work_chunks[i] != NULL) {
            free(work_chunks[i++]);
        }
        free(work_chunks);

        /* Tell workers to stop running */
        for (i = 1; i < sz; ++i)
        {
            int num = 1;
            MPI_Send(&num, 1, MPI_INT, i, DONE_TAG, MPI_COMM_WORLD);
            num_msgs++;
        }

        /* Report result! */
        printf("about to report results\n");
        f->report_results(total_num_chunks, result_array);
        printf("Reporting from master. Num Msgs sent by all processors: %lu\n", num_msgs);

        /* Free memeory of results array */
        for(i = 0; i < total_num_chunks; ++i)
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

            // Receive the next message
            one_work_t *work_chunk = (one_work_t *)malloc(f->work_sz);
            if (!work_chunk) {
                printf("Failed to allocate space for a work chunk on a worker\n");
                exit(1);
            }
            MPI_Status status;
            MPI_Recv(work_chunk, f->work_sz, MPI_CHAR, MASTER, MPI_ANY_TAG, 
                 MPI_COMM_WORLD, &status);

            // If it's a piece of work, do the work and send the result
            if (status.MPI_TAG == WORK_TAG) 
            {
                one_result_t *result = f->do_one_work(work_chunk);
                MPI_Send(result, f->result_sz, MPI_CHAR, MASTER, 
                    RESULT_TAG, MPI_COMM_WORLD);
                free(result);
            } 

            // If it's a "Stop working" message, exit the for loop
            else if(status.MPI_TAG == DONE_TAG)
            {
               i = -1;
                  debug_print("Worker %d says: He terminado!\n", myid);
            }

            // If it's neither of the above messages, we did something wrong. 
            else
            {
                printf("Unexpected Error... Process %d received message with unknown tag.\n", myid);
            }
            free(work_chunk);

      }
    }
    debug_print("Finished Running on processor %d\n", myid);

}

/*==============================================================*/
/* MW_Run Round Robbin style                                    */
/*==============================================================*/
void MW_Run (int argc, char **argv, struct mw_fxns *f){

    // Initialization of parameters
    int sz, myid;
    MPI_Comm_size (MPI_COMM_WORLD, &sz);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid); 

    //Master process
    if(myid == MASTER)
    {
        //Obtain work chunks
        one_work_t **work_chunks = f->create_work_pool(argc, argv);

        //Go through each work chunk
        int i = 0;
        int process_num = 1;
        while (work_chunks[i] != NULL)
        {
            //For send a work chunk to a process using round robin
            one_work_t *work_chunk = work_chunks[i];
            debug_print("MASTER: Sending chunk to process %d out of %d\n", myid, sz);
            MPI_Request send_request;
            MPI_Isend(work_chunk, f->work_sz, MPI_CHAR, process_num, WORK_TAG, MPI_COMM_WORLD, &send_request);
            ++process_num;

            //Make sure it is round robin
            if(process_num < sz)
            {
                process_num = 1;
            }

            ++i;
        }

        //Create a results array to collect the results
        debug_print("MASTER: Finished sending chunks. Collecting results.\n");
        one_result_t **result_array = (one_result_t**)malloc((f->result_sz)*i);

        int j = 0;
        for(j = 0; j < i; j++)
        {
            //Go through each work chunk and collect its result form the process that executed it
            debug_print("MASTER: Collecting result %d out of %d\n", j, i);
            one_result_t *result = (one_result_t*)malloc(f->result_sz);
            MPI_Status status;
            MPI_Recv(result, f->result_sz, MPI_CHAR, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);
            result_array[j] = result;
        }
        
        //Present results (while checking for errors)
        debug_print("MASTER: Finished collecting results. Presenting results.\n");
        if(f->report_results(i, result_array) == 0)
        {
            printf("There was an error in the report_results function.\n");
        }

        //Tell workers to finish running
        for (process_num = 1; process_num < sz; ++process_num)
        {
            one_work_t *work_chunk = (one_work_t*)malloc(f->work_sz);
            MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process_num, DONE_TAG, MPI_COMM_WORLD);
        }
    }
    //For any other process... 
    else
    {
        int j = 0;

        //Iterate until receive message to stop
        while(j == 0)
        {
            //Receive a work chunk
            one_work_t *work_chunk = (one_work_t*)malloc(f->work_sz);
            MPI_Status status;
            debug_print("PROCESS %d: Receiving message from master\n", myid);
            MPI_Recv(work_chunk, f->work_sz, MPI_CHAR, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            //If work_tag then work and send result
            if (status.MPI_TAG == WORK_TAG)
            {
                 debug_print("PROCESS %d: The message was a work chunk!\n", myid);
                 one_result_t *result = f->do_one_work(work_chunk);
                 MPI_Request send_request;
                 MPI_Isend(result, f->result_sz, MPI_CHAR, MASTER, RESULT_TAG, MPI_COMM_WORLD, &send_request);

            }
            //If end tage then simply finish loop and exit
            else
            {
                debug_print("PROCESS %d: The message was to end!\n", myid);
                j = 1;
            }
        } 
    }

    debug_print("PROCESS %d: Finished execution!\n", myid);

}






