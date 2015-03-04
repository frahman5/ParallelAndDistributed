#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "api.h"
/* A short program to calculate 1 + 2 + ... + 100
using a master-worker paradigm */

#define WORK_CHUNKS     10

// this is hardcoded to divide the work into 10 work chunks
// we should make that a variable later
one_work_t **make_work(int argc, char **argv) {
    one_work_t **work_array;
    work_array = (one_work_t **)malloc(10 * sizeof(one_work_t *));
    int i;
    int array_elem = 1; // first array element
    for (i = 0; i < 10; i++) {
        one_work_t *w = (one_work_t *)malloc(sizeof(one_work_t));
        w->sz = 10;
        int j;
        for (j = 0; j < w->sz; j++) {
            w->array[j] = array_elem++;
        }
        work_array[i] = w;
    }

    return work_array;
}

one_result_t *do_work(one_work_t* work)
{
    // calculate the result
    int answer = 0;
    int i;
    for (i = 0; i < work->sz; i++) {
        answer += work->array[i];
    }

    // pack the result up and return it
    one_result_t *result = (one_result_t *)malloc(sizeof(one_result_t));
    result->array[0] = answer;
    result->sz = 1;
    result->status = 0;

    return result;
}

// struct one_result {
//     int array[1];
//     unsigned long long sz;
//     int status;
// } r;

// take an array of results and report the final result
int report(int sz, one_result_t *result_array) {
    int i;
    int result = 0;
    for (i = 0; i < sz; i++) {
        result += result_array[i].array[0];
    }

    printf("Result of computation: %d\n", result);

    return 0;
}


int main (int argc, char **argv) {

    //printf("Hello\n");
    
    // Create the api object and give it all its fields
    struct mw_fxns mw;
    mw.create_work_pool = make_work;
    mw.do_one_work = do_work;
    mw.report_results = report;
    mw.work_sz = sizeof(one_work_t);
    mw.result_sz = sizeof(one_result_t);
    mw.chunks_num = WORK_CHUNKS;

    //Initialize MPI
    MPI_Init (&argc, &argv);

    // run the program
    MW_Run (argc, argv, &mw);

    MPI_Finalize();
    

    return 0;
}


/*
Questions:
  - Are all processes loading mw? (print hello 4 times) Technically this is not an issue since it is just pointers to functions
*/

 //  one_work_t **work_array;
   //  work_array = (one_work_t **)malloc(num_works * sizeof(one_work_t*));
   //  int i;
   //  for (i = 0; i < num_works; i++) {
   //       one_work_t *w = (one_work_t*)malloc(sizeof(one_work_t));
   //       w->sz = 3+i;
   //       w->array = (int*)malloc((w->sz)*sizeof(int));
   //       int j;
   //       for(j = 0; j < w->sz; ++j)
   //       {
   //          w->array[j] = i;
   //       }
   //       work_array[i] = w;
   //      //w->array = array;
   //      work_array[i] = w;
   // }
   //  // printf("parameter received: %d\n", num_works);