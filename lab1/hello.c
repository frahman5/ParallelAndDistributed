#include <stdio.h>
#include <mpi.h>

int main (int argc, char **argv)
{
  int sz, myid;

  MPI_Init (&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  printf ("Hello, I am %d of %d processors!\n", myid, sz);

  MPI_Finalize ();
  exit (0);
}

