## Make MW_Run_2 tolerate failing workers
    -> If Faiyam has time, moduralize the master code

## Make MW_Run_1 tolerate failing masters
- Periodically, the master will write the work_chunk_completion array
and the results array to disk
- Upon failure, the live worker with the lowest id will become the master, 
read the work_chunk_completion array and the results array from disk, 
create the work pool again, and start sending out work chunks
    -> It would be handy if the master code was mosturalized 
- The workers need to listen for messages with MPI_ANY_SOURCE, not just the master
so that when the original master fails, they still receive work chunks

?? Do we need to write data structures about dead workers to disk
    -> Just also write worker_status array to disk, and worker_last_time to disk