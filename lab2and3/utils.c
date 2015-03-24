/* This file contains utility functions that aren't specific to 
our master worker API library */

#include "api.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#ifndef DEBUG
  #define debug_print(M, ...)
#else
  #define debug_print(M, ...) fprintf(stderr, M , ##__VA_ARGS__)
#endif
#define PROB_FAIL       0        // (Contrived...) Probability that a worker will fail
#define DEBUG_FILE      "debug.txt" 

void free_array(void **array, int sz) {
    int i;
    for(i = 0; i < sz; i++) {
        free(array[i]);
    }
    free(array);
}

void logToFile(char* message)
{
    FILE* fp = fopen(DEBUG_FILE, "a+");
    fprintf(fp, message);
    fclose(fp);

}

void logToFileWithInt(char* formatString, int var)
{
    FILE* fp = fopen(DEBUG_FILE, "a+");
    fprintf(fp, formatString, var);
    fclose(fp);

}

void logToFileWithFloat(char *formatString, float var) {
    FILE* fp = fopen(DEBUG_FILE, "a+");
    fprintf(fp, formatString, var);
    fclose(fp);
}

/* Returns 1 if the worker should fail, 0 otherwise */
int random_fail() {

    // Generate a random float between 0 and 1.0
    // logToFileWithInt("Rand Max: %d\n", RAND_MAX);
    float rand_float = (float)rand()/(float)(RAND_MAX);

    // Compare to PROB_FAIL and return
    if (rand_float <= PROB_FAIL) {
        return 1; // the worker should fail
    } else if ((rand_float > PROB_FAIL) && (rand_float <= 1.0)) {
        return 0;
    } else {
        printf("random_fail generated a number not between 0.0 and 1.0!: %f\n", rand_float);
        exit(0);
        return 0;
    }
}


/* This function checks for failed allocations on the heap
  Suggested usage: assert(checkPointer(p, m))) */
int checkPointer(void *pointer, char *error_message) {
    if (pointer == NULL) {
        printf("%s\n", error_message);
        MPI_Finalize();
        exit(0);
    }

    return 1;
}

int F_Send(void *buf, int count, MPI_Datatype datatype, int dest, 
    int tag, MPI_Comm comm, MPI_Request *request) {
    if (random_fail()) {
        logToFile("Uh oh ... A worker failed\n");
        MPI_Finalize();
        exit(0);
        return 0;
    } else {
        return MPI_Send(buf, count, datatype, dest, tag, comm);
    }
}

int initializeIntArray(int* array, int size, int value){
    int j = 0;
    for(j = 0; j < size; ++j)
    {
        array[j] = value;
    }
    return 1;
}

int initializeDoubleArray(double* array, int size, double value){
    int j = 0;
    for(j = 0; j < size; ++j)
    {
        array[j] = value;
    }
    return 1;
}

void waitForUser()
{
    printf("Press any key then enter to continue...\n");
    char m[20];
    scanf("%s", m);
}

// Prints an Array with an associated message
void printIntArray(int *array, int sz, char *message) {
    int j;
    printf("%s\n", message);
    printf("[ ");
    for (j = 0; j < sz; j++) {
        printf("%d ", array[j]);
    }
    printf("]\n");
}

void printDoubleArray(double *array, int sz, char *message) {
    int j;
    printf("%s\n", message);
    printf("[ ");
    for (j = 0; j < sz; j++) {
        printf("%f ", array[j]);
    }
    printf("]\n");

}
void printPointerArray(void **array, int sz, char *message) {
    int j;
    printf("%s\n", message);
    printf("[ ");
    for (j = 0; j < sz; j++) {
        printf("%p ", array[j]);
    }
    printf(" ]");
}

// THis may not print all public attributes, but it prints at least 3 of them!
void printMPIStatus(MPI_Status status, char *message) {

    FILE *fp;
    fp = fopen("debug.txt", "a");

    fprintf(fp, "\n *** PRINTING MPI STATUS ***\n");
    fprintf(fp, "%s\n", message);
    fprintf(fp, "MPI_SOURCE: %d\n", status.MPI_SOURCE);
    fprintf(fp, "MPI_TAG: %d\n", status.MPI_TAG);
    fprintf(fp, "MPI_ERROR: %d\n", status.MPI_ERROR);

    fclose(fp);

}

void cleanMPIStatus(MPI_Status *status) {

    status->MPI_SOURCE = 0;
    status->MPI_TAG = 0;
    status->MPI_ERROR = 0;
}