#include <stdio.h>
#include <mpi.h>
// #include <ctime>

// using namespace std;

#define NUM_TESTS   10          // num iterations of test
#define SEND        0           // process sending message
#define RECV        1           // process receiving message
#define TAG_TEST    4           // Indicates this message is being timed
#define TAG_ACK     5           // Indicates this message is to acknowledge receipt of a test message

int main (int argc, char **argv)
{
  // Initialization of parameters
  int sz, myid;
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);


  // Program begins for each process
  if(sz < 3)
  {
    printf("Too few processes to run latency testing. Exiting program.\n");
    MPI_Finalize ();
    exit(1);
  }

  if(myid == 0)
  {
      int array_time[NUM_TESTS];
      int i;
      for(i = 0; i < NUM_TESTS; ++i)
      {
            printf("we ran the for loop for myid == 0");
            // Log start time of message send
            // time_t current_time = time(0);
            // array_time[i] = current_time;

            // // send test message
            // MPI_Send(&i, sizeof(int), MPI_INT, 
            //     RECV, TAG_TEST, MPI_COMM_WORLD);

            // // receieve acknowledgement of successful test message
            // int recv_data = -1;
            // MPI_Status status;
            // MPI_Recv(&recv_data, sizeof(int), MPI_INT, 
            //     RECV, TAG_ACK, MPI_COMM_WORLD, &status);

      }
  }
  printf ("Hello, I am %d of %d processors!\n", myid, sz);

  MPI_Finalize ();
  exit (0);
}