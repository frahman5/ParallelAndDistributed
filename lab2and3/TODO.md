MAJOR PRIORITY: We have a problem with receiving the last chunk of work. It seems like when
you try to Irecv it, it doesn't work. If you try to both Irecv and Recv it, it doesn't work. 
But if you ONLY Recv it, it seems to work. It could be something else. We need to figure that out. 
    -> After you figure that out, make sure the results are right, and then ....

0. Clean up declarations right before main master loop
0.1: Modularize before MW_Run_2
0.2: Update the "tell workers to stop running" part to account for dead workers
0.3: Think about what other data structures we now need to free

1. Find a way to see if a worker is dead
2. Resend unfinished worker work if a worker is dead
