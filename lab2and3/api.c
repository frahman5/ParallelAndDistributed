#include "api.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define MASTER              0                   // Master process ID
#define DONE_TAG            0                   // This is the master telling the worker it can stop
#define UPDATE_MASTER_TAG   999999999           // Tag to update master in every process
#define DEATH_INTERVAL      5                   // If a worker hasn't sent anything in these seconds he is dead                                    
#define MASTER_STATE_FILE   "master_state.txt"  // Name of the file where the master's last state is stored
#define MASTER_DEATH_INTERVAL  4                // If a master hasn't sent nything in these seconds he is dead
#define WORKER_STATUS_INDEX_TO_TRUE_WORKER_SOURCE(i)    i + 1     

void findNextLiveWorker(int *worker_status, int *process_num, int sz, int master_id) {

    // make sure we're not sending a message to a dead worker, or to ourselves
    while ( (worker_status[*process_num] == 0) || (*process_num == master_id) ) {
        (*process_num)++;
        assert(resetProcessNum(process_num, sz, master_id));
    }
}

int findNextMaster(int *worker_status, int sz, int master_id) {
    int process_num = master_id + 1;
    while (worker_status[process_num] == 0) {
        process_num++;
        assert(resetProcessNum(&process_num, sz, master_id));
    }
    return process_num;
}

int updateResults(int work_chunk_index, one_result_t** result_array, 
    int *result_index, one_result_t* result, int* work_chunk_completion, double* worker_last_time, int worker_index) {

    //Mark the time at which the corresponding worker sent us a result
    worker_last_time[worker_index] = MPI_Wtime();
    // printDoubleArray(worker_last_time, worker_index + 1, "Worker_Last_time array");

    //Store result and increment the index
    if(work_chunk_completion[work_chunk_index] == 1)
    {
        // logToFileWithInt("repeated result work chunk index: %d\n", work_chunk_index);
        return 1;
    }
    
    result_array[ (*result_index)++ ] = result;
    
    //Mark the corresponding work chunk as completed
    work_chunk_completion[work_chunk_index] = 1;

    return 1;
}

// exit the program if all our workers are dead, else return 1
int checkForLiveWorkers(int *worker_status, int num_processes, int master_id) {
    int we_have_live_workers = 0;
    int j;

    for(j = master_id; j < num_processes - 1; j++) {
        we_have_live_workers += worker_status[j];
    }
    if (we_have_live_workers == 0) {
        printf("All the workers are dead! :(. This master is quitting ... \n");
        MPI_Finalize();
        exit(0);
    }

    return 1;
}

/* for round-robbin master worker, reset the process_num to 1
 if it exceeds the total number of worker processes. */
int resetProcessNum(int *process_num, int total_num_processes, int master_id) {
    if (*process_num == total_num_processes)
    { 
        assert (master_id < total_num_processes - 1);
        *process_num = master_id + 1;        
    } 
    return 1;
}

//Returns the len of a NULL terminted array of one_work_t
int len(one_work_t **array) {
    int num = 0;
    while(array[num]) {
        num++;
    }

    return num;
}

//Returns the lowest index of a work chunk in the array that hasnt been completed, else -1
int workLeftToDo(int *work_chunk_completion, int num_work_chunks) {
    int i;
    for (i = 0; i < num_work_chunks; ++i)
    {
        if(work_chunk_completion[i] == 0)
            return i;
    }
    return -1;
}


void initalizeMaster(int argc, char **argv, struct mw_fxns *f, int sz, 
    one_work_t ***work_chunks, int *num_work_chunks, int **work_chunk_completion, 
    int **worker_status, double **worker_last_time, double *cur_time, 
    one_result_t ***result_array) {

    // Obtain and count work chunks
    *work_chunks = f->create_work_pool(argc, argv);
    *num_work_chunks = len(*work_chunks);

    // Create a data structure to keep track of whether or not workers are alive
    // and what work we might need to redo if one dies
    *work_chunk_completion = (int *)malloc(sizeof(int) * *num_work_chunks);
    *worker_status = (int *)malloc(sizeof(int) * (sz - 1));
    assert (checkPointer(*work_chunk_completion, "work_chunk_completion failed to allocate on heap"));
    assert (checkPointer(worker_status, "worker_status failed to allocate on heap"));
    initializeIntArray(*work_chunk_completion, *num_work_chunks, 0);   // all work chunks are incomplete at start
    initializeIntArray(*worker_status, sz - 1, 1);   // all workers are alive at start

    // For keeping track of dead workers
    *cur_time = MPI_Wtime();
    *worker_last_time = (double *)malloc((sz-1) * sizeof(double)); // array[i] = last time ith + 1 process sent result
    assert(checkPointer(*worker_last_time, "worker_last_time failed to allocate on heap"));
    initializeDoubleArray(*worker_last_time, sz - 1, *cur_time);

    *result_array = (one_result_t **)malloc((f->result_sz) * *num_work_chunks);
    assert(checkPointer(*result_array, "Failed to allocate result array while collecting results"));
}

// This is to be called from the master, to tell all the workers to stop working
void restWorkers(int sz, struct mw_fxns *f, int *worker_status, int master_id) {

    int process_num;
    for (process_num = master_id + 1; process_num < sz; ++process_num) {

        // if the worker is alive, tell him to quit yo, else fuggedaboutit
        if (worker_status[process_num - 1] == 1) {
            one_work_t *work_chunk = (one_work_t*)malloc(f->work_sz);
            assert(checkPointer(work_chunk, "Failed to allocate work_chunk while sending a done message"));
            MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process_num, DONE_TAG, MPI_COMM_WORLD);
            free(work_chunk);
        }
    }
}

// This is to update the data structure that keeps track of dead workers
void checkForDeadWorkers(double *worker_last_time, int **worker_status, int sz, int master_id) {
    double cur_time = MPI_Wtime();
    int t;
    for (t = master_id; t < (sz-1); ++t)
    {
        if((cur_time - worker_last_time[t]) > DEATH_INTERVAL) { 
            (*worker_status)[t] = 0;
        }
    }

}

/* Receive a result in the master, and return the id of the worker who sent you that result */
int receiveResult(struct mw_fxns *f, MPI_Status probe_status, 
    one_result_t **result_array, int *result_index, 
    int *work_chunk_completion, double *worker_last_time, 
    int *received) {

    MPI_Status recv_status;

    // Go get the message waiting for us in our mailbox
    one_result_t *result = (one_result_t *)malloc(f->result_sz);
    assert(checkPointer(result, "Failed to allocate a result while collecting worker results"));
    MPI_Recv(result, f->result_sz, MPI_CHAR, probe_status.MPI_SOURCE, 
        probe_status.MPI_TAG, MPI_COMM_WORLD, &recv_status);

    // Make sure we got what we expected
    assert(probe_status.MPI_SOURCE == recv_status.MPI_SOURCE);
    assert(probe_status.MPI_TAG == recv_status.MPI_TAG);

    // Update the result_array, result_index, and work_chumk_completion array
    updateResults(recv_status.MPI_TAG - 1, result_array, result_index, 
            result, work_chunk_completion, worker_last_time, recv_status.MPI_SOURCE - 1);

    // Reset the received tag
    *received = 0; 

    return recv_status.MPI_SOURCE;
}

//Updates the master's current state in a file in disk
void UpdateMasterState(int *work_chunk_completion, one_result_t **result_array, int num_work_chunks,
    int result_index, struct mw_fxns *f)
{   
    //Open the master's last state file
    FILE *fp;

    //Using w ensures the file contents are cleared! Maybe need to manage this
    fp = fopen(MASTER_STATE_FILE, "wb"); 
    assert(checkPointer(fp, "failed to open a file pointer in RetrieveMasterState\n"));

    //Write result_index  to file
    fwrite(&result_index, sizeof(int), 1, fp);

    //Write work_chunk_completion array to file
    fwrite(work_chunk_completion, sizeof(int), num_work_chunks, fp);

    //Write results array to file
    int i = 0;
    for (i = 0; i < result_index; i++) {
        fwrite(result_array[i], f->result_sz, 1, fp);
    }
    
    fclose(fp);
}

//Retrieves the master's current state from a file in disk
void RetrieveMasterState(int *work_chunk_completion, one_result_t **result_array, int num_work_chunks,
  int *result_index, struct mw_fxns *f)
{
    //Open the master's last state file
    FILE *fp;

    //Using w ensures the file contents are cleared! Maybe need to manage this
    fp = fopen(MASTER_STATE_FILE, "rb"); 
    assert(checkPointer(fp, "failed to open a file pointer in RetrieveMasterState\n"));

    //Read result_index  to file
    fread(result_index, sizeof(int), 1, fp);

    //Read work_chunk_completion array to file
    fread(work_chunk_completion, sizeof(int), num_work_chunks, fp);

    //Read results array to file
    int i;
    one_result_t *result;
    for (i = 0; i < *result_index; i++) {
        result = (one_result_t *)malloc(f->result_sz);
        assert(checkPointer(result, "Failed to allocate a result while retrieving master state"));
        fread(result, f->result_sz, 1, fp);
        result_array[i] = result;
    }

    fclose(fp);
}


void alertWorkersNewMaster(int *worker_status, int size, struct mw_fxns *f, int new_master, int current_master)
{
    int i;
    for(i = current_master; i < size; ++i)
    {
        if (worker_status[i] == 1)
        {
            assert(WORKER_STATUS_INDEX_TO_TRUE_WORKER_SOURCE(i) >= new_master);
            MPI_Send(&new_master, 1, MPI_INT, WORKER_STATUS_INDEX_TO_TRUE_WORKER_SOURCE(i),
                UPDATE_MASTER_TAG, MPI_COMM_WORLD);   
        }
    }
}

void runRoundRobbinMaster(int argc, char **argv, struct mw_fxns *f, int sz, int myid, int called_from_worker) {

    printf("Running MW Round-Robbin style\n");

    // Stuff to keep track of work chunks we've sent out, and whether or not workers are dead
    one_work_t **work_chunks;
    int num_work_chunks, *work_chunk_completion, *worker_status; 
    double *worker_last_time, cur_time;

    // Stuff to store results
    one_result_t *result;
    one_result_t **result_array;

    // Initalize them all!!!
    initalizeMaster(argc, argv, f, sz, &work_chunks, &num_work_chunks, 
        &work_chunk_completion, &worker_status, &worker_last_time, &cur_time, 
        &result_array);

    /* Initialize variables that are used throughout */
        // For tracking sent work chunks and received completed work chunks
    int work_chunk_iterator = 0;
    int process_num = 1;
    int result_index = 0;
    int received = -10;
    MPI_Status probe_status;                               // contains metadata about probed messages
        // Who takes over if i fail
    int previous_designated_master = 0;
    int designated_master = 1;

    //Update the master's state for the first time
    if(called_from_worker == 0)
    {
        // find the the available master
        UpdateMasterState(work_chunk_completion, result_array, num_work_chunks, result_index, f);
    }
    else
    {
        RetrieveMasterState(work_chunk_completion, result_array, num_work_chunks, &result_index, f);
    }
        
    printf("When entering the main master loop ,result index, num_work_chunks: %d, %d\n", result_index, num_work_chunks);
    while (result_index < num_work_chunks) 
    {
        // Make master fail
        if(result_index == 2 && called_from_worker == 0)
        {
            printf("MASTER FAILED!!!!\n");
            logToFileWithInt("When master failed, the incumbent master was: %d\n", designated_master);
            MPI_Finalize();
            exit(0);
        }

        // if(myid == 1)
        // {
        //     printf("MASTER FAILED!!!!\n");
        //     // waitForUser();
        //     MPI_Finalize();
        //     exit(0);
        // }
        // Have we received anything?
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &received, &probe_status);
         
        //Receive results if possible, and update the master state
        if (received == 1) {
            receiveResult(f, probe_status, result_array, &result_index, 
                work_chunk_completion, worker_last_time, &received);  

            UpdateMasterState(work_chunk_completion, result_array, num_work_chunks, result_index, f);     
        }

        // Check for dead workers, and if need be update the workers on who the new master is
        checkForDeadWorkers(worker_last_time, &worker_status, sz, myid);
        designated_master = findNextMaster(worker_status, sz, myid);
        if (previous_designated_master != designated_master) 
        {
            previous_designated_master = designated_master;
            alertWorkersNewMaster(worker_status, sz-1, f, designated_master, myid);
        }
            

        // check if we have live workers. If not, exit the program
        assert (checkForLiveWorkers(worker_status, sz, myid));

        // If we DO have live workers, send them some work!
        int lowest_undone_work_index = workLeftToDo(work_chunk_completion, num_work_chunks);
        if (work_chunks[work_chunk_iterator] != NULL || lowest_undone_work_index != -1) {

            // Send a work chunk to a process using round robin
            one_work_t *work_chunk;
            int tag;

            if(work_chunks[work_chunk_iterator] != NULL)
            {
                tag = work_chunk_iterator + 1;
                work_chunk = work_chunks[work_chunk_iterator++];
            }
            else
            {
                work_chunk = work_chunks[lowest_undone_work_index];
                tag = lowest_undone_work_index + 1;
            }

            // find the the next live worker
            findNextLiveWorker(worker_status, &process_num, sz, myid);  

            // Make an ISend because if process is still working when sending for some reason it fails with Send
            MPI_Request master_send_request;
            MPI_Isend(work_chunk, f->work_sz, MPI_CHAR, process_num, tag, MPI_COMM_WORLD, &master_send_request);
            process_num++;
            // Reset the process id if necessary, ensuring round robbin-ness.
            resetProcessNum(&process_num, sz, myid);
        }

    }

    // Present results (while checking for errors)
    if (f->report_results(num_work_chunks, result_array) == 0) {
        printf("There was an error in the report_results function.\n");
    }

    // RetrieveMasterState(work_chunk_completion, result_array, num_work_chunks, &result_index, f);

    // Tell workers to finish running
    restWorkers(sz, f, worker_status, myid);

    // What is the state of the world according to the master at the end of execution?
    printIntArray(worker_status, sz-1, "Worker status at end of master: ");

    // free up data structures
    free_array((void **) result_array, num_work_chunks);
    free_array((void **) work_chunks, num_work_chunks);
    free(work_chunk_completion);
    free(worker_status);
    free(worker_last_time);
}

// void runDynamicMaster(int argc, char **argv, struct mw_fxns *f, int sz) {

//     printf("Running MW with dynamic work allocation\n");

//     // Initialize all the necessary variables
//     one_work_t **work_chunks;                   /* Work chunk and worker trackers */
//     int num_work_chunks, *work_chunk_completion, *worker_status; 
//     double *worker_last_time, cur_time;
//     one_result_t *result;                       /* Stuff to store results */
//     one_result_t **result_array;
//     initalizeMaster(argc, argv, f, sz, &work_chunks, &num_work_chunks, 
//         &work_chunk_completion, &worker_status, &worker_last_time, &cur_time, 
//         &result_array);

//     // For tracking sent work chunks and received completed work chunks
//     int work_chunk_iterator = 0;
//     int result_index = 0;
//     int received = -10;
//     int send_flag = 0;
//     MPI_Status probe_status;

//     /* Send out the first batch of work to workers round-robbin style */
//     logToFileWithInt("Total Number of Chunks: %d\n", num_work_chunks);
//     int process_num;
//     for (process_num = 1; process_num < sz; process_num++) {

//         // Send the next work chunk
//         one_work_t *work_chunk = work_chunks[work_chunk_iterator];
//         int tag;
//         if (work_chunk) {
//             tag = work_chunk_iterator++ + 1;
//             MPI_Send(work_chunk, f->work_sz, MPI_CHAR, process_num,
//                 tag, MPI_COMM_WORLD);
//         } else {
//             printf("Exiting the initial round robbin for loop in master\n");
//             process_num = sz;                           // exit the for loop
//         }
//     }

//     /* Send out rest dynamically */
//     while (result_index < num_work_chunks) 
//     {

//         // Have we received anything?
//         MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &received, &probe_status);
         
//         //Receive results if possible
//         if (received == 1) {
//             process_num = receiveResult(f, probe_status, result_array, &result_index, 
//                 work_chunk_completion, worker_last_time, &received);
//             send_flag = 1;          
//         }

//         // If there's unfinished work, send it out
//         if (send_flag) {
//             int lowest_undone_work_index = workLeftToDo(work_chunk_completion, num_work_chunks);
//             if (work_chunks[work_chunk_iterator] != NULL || lowest_undone_work_index != -1) {

//                 // Send a work chunk to the process that just sent us work
//                 one_work_t *work_chunk;
//                 int tag;
//                 if(work_chunks[work_chunk_iterator] != NULL) {
//                     tag = work_chunk_iterator + 1;
//                     work_chunk = work_chunks[work_chunk_iterator++];
//                 }
//                 else {
//                     work_chunk = work_chunks[lowest_undone_work_index];
//                     tag = lowest_undone_work_index + 1;
//                 }

//                 // Make an ISend because if process is still working when sending for some reason it fails with Send
//                 MPI_Request master_send_request;
//                 MPI_Isend(work_chunk, f->work_sz, MPI_CHAR, process_num, tag, MPI_COMM_WORLD, &master_send_request);
//             }
//             send_flag = 0;
//         }
        
//         // check if we have live workers. If not, exit the program
//         checkForDeadWorkers(worker_last_time, &worker_status, sz);
//         printIntArray(worker_status, sz-1, "Worker status: ");
//         assert (checkForLiveWorkers(worker_status, sz));

//     }

//     // Present results (while checking for errors)
//     if (f->report_results(num_work_chunks, result_array) == 0) {
//         printf("There was an error in the report_results function.\n");
//     }

//     // Tell workers to finish running
//     restWorkers(sz, f, worker_status);

//     /* Free things that are on the heap */
//     free(work_chunk_completion);
//     free(worker_status);
//     free(worker_last_time);
//     free_array((void **)work_chunks, num_work_chunks);
//     free_array((void **)result_array, num_work_chunks);

// }

int isMasterDead(double current_time, double last_recv_time) {
    return (last_recv_time != 0) && (current_time - last_recv_time > MASTER_DEATH_INTERVAL);
}


void runWorker(int argc, char **argv, int sz, struct mw_fxns *f, int myid) {
    int j = 0;
    int new_master = MASTER;

    double last_recv_time = 0;
    double current_time;
    int received = 0;
    MPI_Status probe_status;

    //Iterate until receive message to stop
    while(j == 0)
    {
        current_time = MPI_Wtime();

        // Check if the master is dead
        if(isMasterDead(current_time, last_recv_time))
        {
            last_recv_time = 0;

            //If I am the new master, be the new master
            if(myid == new_master)
            {
                runRoundRobbinMaster(argc, argv, f, sz, myid, 1);
                j = 1;
            }
        }

        // Have we received anything?
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &received, &probe_status);

        // If necessary update the new master
        if((received == 1) && (probe_status.MPI_TAG == UPDATE_MASTER_TAG))
        {

            MPI_Status status;
            MPI_Recv(&new_master, 1, MPI_INT, probe_status.MPI_SOURCE, probe_status.MPI_TAG, 
                MPI_COMM_WORLD, &status);                
            last_recv_time = MPI_Wtime();
            received = 0;
        }
        else if(received == 1)
        {
            // Make sure we are either doing work or being told to stop
            assert(probe_status.MPI_TAG == DONE_TAG || probe_status.MPI_TAG > 0);
            
            //Receive a work chunk
            one_work_t *work_chunk = (one_work_t*)malloc(f->work_sz);
            assert(checkPointer(work_chunk, "Failed to allocate a work_chunk on a worker"));

            MPI_Status status;
            MPI_Recv(work_chunk, f->work_sz, MPI_CHAR, probe_status.MPI_SOURCE, probe_status.MPI_TAG, 
                MPI_COMM_WORLD, &status);                
            
            //Update last_recv time
            last_recv_time = MPI_Wtime();

            //If work_tag then work and send result
            if (status.MPI_TAG > 0)
            {
                int masters_work_tag = status.MPI_TAG;
                one_result_t *result = f->do_one_work(work_chunk);
                MPI_Request send_request;
                F_Send(result, f->result_sz, MPI_CHAR, probe_status.MPI_SOURCE, masters_work_tag, MPI_COMM_WORLD, &send_request);
                free(result); free(work_chunk);
                received = 0;
            }
            //If end tag then simply finish loop and exit
            else
            {
                assert(status.MPI_TAG == DONE_TAG);
                j = 1;
            }

        }
        
    } 
}

/*==============================================================*/
/* Master MW_Run function.                                      */
/*==============================================================*/
void MW_Run (int argc, char **argv, struct mw_fxns *f, int style) {

    // Make sure the user asked for a valid style of master worker
    assert(style == 1 || style == 2);

    // Initialization of parameters
    int sz, myid;
    MPI_Comm_size (MPI_COMM_WORLD, &sz);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid); 
    if (myid == 0) {
        logToFile("\n\n**New Run**\n\n");
    }

    // Seed the pseudo-random number generator
    srand(myid * (unsigned int)time(NULL));

    // Master Round Robbin
    if( (myid == MASTER) && (style == 1) )
    {
        runRoundRobbinMaster(argc, argv, f, sz, myid, 0);
    } 

    // Master Dynamic
    else if ( (myid == MASTER) && (style == 2) ) {
        // runDynamicMaster(argc, argv, f, sz);
    }
    // For any other process... 
    else
    {

        runWorker(argc, argv, sz, f, myid);
    }
}

