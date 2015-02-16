#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif

#define NUM_TESTS   100          // num iterations of test
#define PACKET_STEP 100           // step size of tests (i.e 5 packets, then 10 packets, etc)
#define NUM_MSGS    10          // num messages to send for every iteration of the test
#define SEND_ID     0           // process sending message
#define RECV_ID     1           // process receiving message
#define TAG_TEST    4           // Indicates this message is being timed
#define TAG_ACK     5           // Indicates this message is to acknowledge receipt of a test message

void print_array(double *array, int size, char *array_name) {
  int i;
  for(i = 0; i < size; i++) {
    printf("%s elem %d: %f\n", array_name, i, array[i]);
  }
}

int main (int argc, char **argv)
{
  // Initialization of parameters
  int sz, myid;
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);


  // Program begins for each process
  if(sz < 2)
  {
    printf("Too few processes to run latency testing. Exiting program.\n");
    MPI_Finalize ();
    return 0;
  }

  if(myid == SEND_ID)
  {
      int packet_size;

      // for(packet_size = PACKET_STEP; packet_size <= PACKET_STEP * NUM_TESTS; packet_size += PACKET_STEP) 
     for(packet_size = PACKET_STEP * NUM_TESTS; packet_size >= PACKET_STEP; packet_size -= PACKET_STEP) 
      {
        int i;
        for(i = 0; i < NUM_MSGS; ++i)
        {
            
            // create test message
            double *time_array = (double *)malloc(packet_size * sizeof(double));
            if (!time_array) {
              printf("Failed to allocate time_array on processor %d", myid);
              MPI_Finalize();
              return 0;
            }
            int j;
            for(j = 0; j < packet_size; j++) {
              time_array[j] = 0.0;
            }

            // Log start time of message send
            time_array[0] = MPI_Wtime();

            // Send test message
            MPI_Send(time_array, packet_size, MPI_DOUBLE, RECV_ID, TAG_TEST, MPI_COMM_WORLD);
            debug_print("SENDER: Message %d sent - %f.\n", i, current_time);

            // clean heap
            free(time_array);

            // Receieve acknowledgement of successful test message
            int acknowledgement;
            MPI_Status status;
            MPI_Recv(&acknowledgement, 1, MPI_INT, RECV_ID, TAG_ACK, MPI_COMM_WORLD, &status);
        }       
      }


      debug_print("SENDER: Execution finished.\n");
  }
  else if(myid == RECV_ID)
  {
      double results_array[NUM_TESTS][2];
      int packet_size;

      // for(packet_size = PACKET_STEP; packet_size <= PACKET_STEP * NUM_TESTS; packet_size += PACKET_STEP) 
      for(packet_size = PACKET_STEP * NUM_TESTS; packet_size >= PACKET_STEP; packet_size -= PACKET_STEP) 
      {
        int i;
        double message_latency = 0.0;

        // Calculate latency for this packet size
        for(i = 0; i < NUM_MSGS; ++i)
        {
            // Receive message 
            double *send_time_array = (double *)malloc(packet_size * sizeof(double));
            if (!send_time_array) {
              printf("Failed to allocate send_time_array on processor %d", myid);
              MPI_Finalize();
              return 0;
            }
            MPI_Status status;
            MPI_Recv(send_time_array, packet_size, MPI_DOUBLE, SEND_ID, TAG_TEST, MPI_COMM_WORLD, &status);

            // Check that we got the right thing
            debug_print("RECEIVER: Message %d received - %f.\n", i, send_time_array[0]);
            #ifdef DEBUG 
                print_array(send_time_array, packet_size, "send_time_array");
            #endif
            // Compare how much time the message took between send and receive time
            double receive_time = MPI_Wtime();
            double result_time = receive_time - send_time_array[0];
            debug_print("RECEIVER: Time elapsed -  %f seconds.\n", result_time);
            message_latency += result_time;
            free(send_time_array);

            // Send acknowledgement to sender
            int acknowledgement = 1;
            MPI_Send(&acknowledgement, 1, MPI_INT, SEND_ID, TAG_ACK, MPI_COMM_WORLD);
            debug_print("RECEIVER: Acknowledgement sent.\n");
        }

        debug_print("    Packet Size, Message latency: %d, %f\n", packet_size, message_latency/NUM_MSGS);

        // Update results_array
        int num_iter = (packet_size / PACKET_STEP) - 1;
        results_array[num_iter][0] = packet_size;
        results_array[num_iter][1] = message_latency/NUM_MSGS;

        debug_print("RECEIVER: Final message latency for packet size=%d - %f.\n", 
          packet_size, message_latency/NUM_MSGS);
      }

      #ifndef DEBUG
        printf("Packet size, Message Latency\n");
        int i;
        for(i = 0; i < NUM_TESTS; i++) 
        {
          printf("%d, %f\n", (int)results_array[i][0], results_array[i][1]);
        }
      #endif

      debug_print("RECEIVER: Execution finished.\n");
  }

  MPI_Finalize ();
  return 0;
}