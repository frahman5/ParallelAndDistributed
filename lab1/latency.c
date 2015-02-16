#include <stdio.h>
#include <mpi.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif

#define NUM_TESTS   10           // num iterations of test
#define SEND_ID     0           // process sending message
#define RECV_ID     1           // process receiving message
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
    return 0;
  }

  if(myid == SEND_ID)
  {
      int i;

      for(i = 0; i < NUM_TESTS; ++i)
      {
          // Log start time of message send
          double current_time = MPI_Wtime();

          // Send test message
          MPI_Send(&current_time, 1, MPI_DOUBLE, RECV_ID, TAG_TEST, MPI_COMM_WORLD);
          debug_print("SENDER: Message %d sent - %f.\n", i, current_time);

          // Receieve acknowledgement of successful test message
          int acknowledgement;
          MPI_Status status;
          MPI_Recv(&acknowledgement, 1, MPI_INT, RECV_ID, TAG_ACK, MPI_COMM_WORLD, &status);
      }

      debug_print("SENDER: Execution finished.\n");
  }
  else if(myid == RECV_ID)
  {
      int i;
      double message_latency = 0.0;

      for(i = 0; i < NUM_TESTS; ++i)
      {

          // Receieve message 
          double send_time;
          MPI_Status status;
          MPI_Recv(&send_time, 1, MPI_DOUBLE, SEND_ID, TAG_TEST, MPI_COMM_WORLD, &status);

          debug_print("RECEIVER: Message %d received - %f.\n", i, send_time);

          // Compare how much time the message took between send and receive time
          double receive_time = MPI_Wtime();
          double result_time = receive_time - send_time;
          debug_print("RECEIVER: Time elapsed -  %f seconds.\n", result_time);
          message_latency += result_time;

          // Send acknowledgement to sender
          int acknowledgement = 1;
          MPI_Send(&acknowledgement, 1, MPI_INT, SEND_ID, TAG_ACK, MPI_COMM_WORLD);
          debug_print("RECEIVER: Acknowledgement sent.\n");

      }

      debug_print("RECEIVER: Final message latency - %f.\n", message_latency/NUM_TESTS);
      debug_print("RECEIVER: Execution finished.\n");

      #ifndef DEBUG
        printf("Final message latency - %f.\n", message_latency/NUM_TESTS);
      #endif
  }

  MPI_Finalize ();
  return 0;
}