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
  - Check the create_work function to see if it suceeded or not 
  */

/****

	This function does the following:
		- Receives api object 
		- Uses the create_work_pool function to get the array of workers
		- Goes through the array and executes the necessary work for each
		- Prints the results
	
****/

// Runs master worker, sending out the first batch of work round-robbin style
// and then only sending other chunks once a worker says he's done
// void MW_Run_2 (int argc, char **argv, struct mw_fxns *f){
  
//     // Counts number of messages sent (for granularity)
//     unsigned long num_msgs = 0;

//     // Initialization of parameters
//     int sz, myid;
//     MPI_Comm_size (MPI_COMM_WORLD, &sz);
//     MPI_Comm_rank (MPI_COMM_WORLD, &myid); 

//     // master process sets up calculation
//     if(myid == MASTER)
//     {
//       printf("Number of processors: %d\n", sz);
//       /* Create all the work to be done */
//       one_work_t **work_chunks = f->create_work_pool(argc, argv);

//       /* Send out the first batch of work to workers round-robbin style */
//       int process_num;
//       int work_chunk_index = 0;
//       int work_chunk_count = 0;
//       for (process_num = 1; process_num < sz; process_num++) {

//           // Send the next work chunk
//           one_work_t *work_chunk = work_chunks[work_chunk_index++];
//           if (work_chunk) {
//               printf("Sending Process %d out of %d their first work chunk. Work chunk count: %d\n", process_num, sz, work_chunk_count);
//               MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process_num,
//                 WORK_TAG, MPI_COMM_WORLD);
//               work_chunk_count++;
//               num_msgs++;
//           } else {
//               process_num = sz; // exit the for loop
//           }
//           debug_print("Nothing went while sending work chunk %d\n", i);
//       }
//       printf("Work chunk count after sending work on initial round robbin pass: %d\n", work_chunk_count);

//       /* Free up work chunks: This was allocated in the user's do_work function */
//       int i = 0;
//       while(work_chunks[i] != NULL) {
//           free(work_chunks[i++]);
//       }
//       free(work_chunks);

//       //We receive results..
//       one_result_t **result_array = (one_result_t**)malloc(sizeof(one_result_t*)*work_chunk_count);
//       if (!result_array) {
//           printf("MW library failed to allocate result_array on the heap\n");
//           exit(1);
//       }
//       for(i = 0; i < work_chunk_count; ++i)
//       {
//           printf("getting result %d\n", i);
//           one_result_t *result = (one_result_t*)malloc((f->result_sz));
//           if (!result) {
//               printf("MW Library failed to allocate a result struct on the heap\n");
//               exit(1);
//           }
//           MPI_Status status;

//           MPI_Recv(result, f->result_sz, MPI_CHAR, MPI_ANY_SOURCE, RESULT_TAG, 
//             MPI_COMM_WORLD, &status);
//           num_msgs++;

//           result_array[i] = result;
//       }

//       //Tell workers to stop running
//       for (i = 1; i < sz; ++i)
//       {
//           int num = 1;
//           MPI_Send(&num, 1, MPI_INT, i, DONE_TAG, MPI_COMM_WORLD);
//           num_msgs++;
//       }

//       //Report result!
//       printf("about to report results\n");
//       f->report_results(work_chunk_count, result_array);
//       printf("Reporting from master. Num Msgs sent by all processors: %lu\n", num_msgs);

//       // Free memeory of results array
//       for(i = 0; i < work_chunk_count; ++i)
//       {
//           free(result_array[i]);
//       }
//       free(result_array);
//     }

//     // workers do work
//     else
//     {
//       debug_print("Hola, desde processor %d\n", myid);
//       int i = 1;
//       while(i > 0) {
//           one_work_t *work_chunk = (one_work_t *)malloc(f->work_sz);
//           if (!work_chunk) {
//               printf("Failed to allocate space for a work chunk on a worker\n");
//               exit(1);
//           }

//           MPI_Status status;
//           MPI_Recv(work_chunk, f->work_sz, MPI_CHAR, MASTER, MPI_ANY_TAG, 
//             MPI_COMM_WORLD, &status);

//           if (status.MPI_TAG == WORK_TAG) 
//           {
//               one_result_t *result = f->do_one_work(work_chunk);
//               MPI_Send(result, f->result_sz, MPI_CHAR, MASTER, 
//                 RESULT_TAG, MPI_COMM_WORLD);
//               free(result);
//           } 
//           else if(status.MPI_TAG == DONE_TAG)
//           {
//               i = -1;
//               debug_print("Worker %d says: He terminado!\n", myid);
//           }
//           else
//           {
//              printf("Unexpected Error... Process %d received message with unknown tag.\n", myid);
//           }
//           free(work_chunk);

//       }
//     }
//     debug_print("Finished Running on processor %d\n", myid);

// }

/*==============================================================*/
/* MW_Run with Dynamic Process selection                        */
/*==============================================================*/
void MW_Run_2 (int argc, char **argv, struct mw_fxns *f){



    // Initialization of parameters
    int sz, myid;
    MPI_Comm_size (MPI_COMM_WORLD, &sz);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid); 

    int total_number_elements;
    int number_of_processes;
    int number_chunks_per_process;

    one_work_t **work_chunks;
    one_work_t **work_chunks_sub;

    one_result_t **results_array;
    one_result_t **results_array_sub;

    

    if(myid == MASTER)
    {
        work_chunks = f->create_work_pool(argc, argv);

        int total_number_elements = 0;
        while (work_chunks[total_number_elements] != NULL)
        {
            total_number_elements++;
        } //Count number of processes necessary

        if(total_number_elements <= sz)
        {
            number_of_processes = total_number_elements;
            number_chunks_per_process = 1;
        }
        // else if(total_number_elements%sz == 0)
        // {
        //   printf("HERE2");
        //     number_of_processes = sz;
        //     number_chunks_per_process = total_number_elements/sz;
        // }
        // else
        // {
        //   printf("HERE3");
        //     number_of_processes = sz;
        //     while(total_number_elements%number_of_processes != 0)
        //     {
        //         number_of_processes = number_of_processes - 1;
        //         number_chunks_per_process = total_number_elements/sz;
        //     }
        // }

        // 
    }

    results_array = (one_result_t**)malloc((f->result_sz)*total_number_elements);

    //Broadcast number of process and chunks per process to everyone
    MPI_Bcast(&number_of_processes, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&number_chunks_per_process, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&total_number_elements, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

    //Every process waits until they get to this point
    MPI_Barrier(MPI_COMM_WORLD);

    // Create a buffer that will hold a subset of the work_chunks array
    work_chunks_sub = (one_work_t**)malloc(f->work_sz * number_chunks_per_process); 
    
    
    // Scatter the random numbers to all processes
    MPI_Scatter(work_chunks, number_chunks_per_process*f->work_sz, MPI_BYTE, work_chunks_sub, 
      number_chunks_per_process*f->work_sz, MPI_BYTE, MASTER, MPI_COMM_WORLD);


        results_array_sub = (one_result_t**)malloc((f->result_sz)*number_chunks_per_process);
        int i;
        for(i = 0; i < number_chunks_per_process; ++i)
        {
            results_array_sub[i] = f->do_one_work(work_chunks_sub[i]);
        }
    

    // // Gather the results in the master
    // MPI_Gather(results_array_sub, number_chunks_per_process*f->result_sz, MPI_BYTE, results_array, 
    //   number_chunks_per_process*f->result_sz, MPI_BYTE, MASTER, MPI_COMM_WORLD);

    // //Report the results
    // if(myid == 0)
    // {
    //     if(f->report_results(total_number_elements, results_array) == 0)
    //     {
    //         printf("There was an error in the report_results function.\n");
    //     }
    // }

}


/*==============================================================*/
/* MW_Run in Round Robin                                        */
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






