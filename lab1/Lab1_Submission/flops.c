#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif

#define MASTER      0           // Master process ID
#define ARRAY_SIZE  1000000     // Size of arrays for dot product
#define WORK_MSG_1  1           // Array to be sent
#define WORK_MSG_2  2           // Array to be sent
#define RESULT_MSG  3           // The result of a dot product calculation

double dotProduct(double *a1, double *a2, unsigned long long length) {
  unsigned long long i;
  double result = 0.0;
  for(i = 0; i < length; i++) 
  {
    result += (a1[i] * a2[i]);
  }
  
  return result;
}

int main (int argc, char **argv)
{
  // Initialization of parameters
  int sz, myid;
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid); 

  // master process sets up calculation
  if(myid == MASTER)
  {
      // Initalize vectors 
      double *array1 = (double *)malloc(ARRAY_SIZE * sizeof(double));
      double *array2 = (double *)malloc(ARRAY_SIZE * sizeof(double));
      if (!array1 || !array2) 
      {
        printf("Error: Array1 or Array2 failed to allocate with ARRAY_SIZE %d\n", ARRAY_SIZE);
        MPI_Finalize();
        return 0;
      }
      unsigned long long i;
      for(i = 0; i < ARRAY_SIZE; ++i)
      {
          array1[i] = 1.0;
          array2[i] = 1.0;
      }
      
      // If there's only one process, have the master do the work
      if (sz == 1) 
      {
          printf("Executing dot product on the master. only 1 process!\n");
          double start_time = MPI_Wtime();
          double result = dotProduct(array1, array2, ARRAY_SIZE);
          double end_time = MPI_Wtime();
          double FLOPS = (ARRAY_SIZE * 2) / (end_time - start_time);
          printf("FLOPS: %f\n", FLOPS);
 
          MPI_Finalize();
          return 0;
      }

      // Send each process the chunk they are going to calculate
      int process;
      for(process = 1; process < sz; process++) 
      {
        int num_elems = ARRAY_SIZE / (sz - 1);
        int offset = num_elems * (process - 1);

        MPI_Send(array1 + offset, num_elems, MPI_DOUBLE, process, 
          WORK_MSG_1, MPI_COMM_WORLD);
        MPI_Send(array2 + offset, num_elems, MPI_DOUBLE, process, 
          WORK_MSG_2, MPI_COMM_WORLD);
      }

      // Get the start time
      double start_time = MPI_Wtime();

      // Receive the results from each process
      double *results = (double *)malloc((sz - 1) * sizeof(double));
      MPI_Status status;
      for(process = 1; process < sz; process++) 
      {
        int offset = process - 1;
        MPI_Recv(results + offset, 1, MPI_DOUBLE, process,
            RESULT_MSG, MPI_COMM_WORLD, &status);
      }

      // Calculate FLOPS
      double end_time = MPI_Wtime();
      double flops = (ARRAY_SIZE * 2) / (end_time - start_time);

      // Add up all the results
      int j;
      double result = 0.0;
      for(j = 0; j < (sz - 1); j++) 
      {
          result += results[j];
      }
      debug_print("Result of dot product: %f\n", result);
      free(results);

      // print results
      printf("FLOPS: %f\n", flops);
  }

  // workers do calculation
  else {

    // get the work
    int work_length = ARRAY_SIZE / (sz - 1);
    double *work_array_1 = (double *)malloc(work_length * sizeof(double));
    double *work_array_2 = (double *)malloc(work_length * sizeof(double));
    if (!work_array_1 || !work_array_2)
    {
      printf("A Work array failed to allocate on process %d\n", myid);
      MPI_Finalize();
      return 0;
    }
    MPI_Status status1, status2;
    MPI_Recv(work_array_1, work_length, MPI_DOUBLE, MASTER, WORK_MSG_1, 
      MPI_COMM_WORLD, &status1);
    MPI_Recv(work_array_2, work_length, MPI_DOUBLE, MASTER, WORK_MSG_2, 
      MPI_COMM_WORLD, &status2);

    // do the work
    double result = dotProduct(work_array_1, work_array_2, work_length);
    MPI_Send(&result, 1, MPI_DOUBLE, MASTER, RESULT_MSG, MPI_COMM_WORLD);
    free(work_array_1);
    free(work_array_2);
  }

  MPI_Finalize ();
  return 0;
}
