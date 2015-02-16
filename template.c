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
  MPI_Send(pointer_to_buffer, length_of_buffer, MPI_Type, receiver_id, 
    message_tag, communicator__probably_Mpi_comm_world)
  MPI_Recv(pointer_to_recv_buffer, length_of_buffer, MPI_Type, 
    sender_id, message_tag, communicator__probably_Mpi_comm_world, poitner_to_mpi_status_var)
 
  

  MPI_Finalize ();
  return 0;
}
