#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif


int main (int argc, char **argv)
{
  // Initialization of parameters
  int sz, myid;
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  // Send test message
  MPI_Send(time_array, packet_size, MPI_DOUBLE, RECV_ID, TAG_TEST, MPI_COMM_WORLD);
  MPI_Recv(&acknowledgement, 1, MPI_INT, RECV_ID, TAG_ACK, MPI_COMM_WORLD, &status);

 
  

  MPI_Finalize ();
  return 0;
}