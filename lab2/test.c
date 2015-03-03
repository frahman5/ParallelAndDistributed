#include <stdio.h>
#include <stdlib.h>
#include "api.h"
/* A short program to calculate 1 + 2 + ... + 100
using a master-worker paradigm */

struct one_work {
    int array[10];
    unsigned long long sz;
} w;

struct one_result {
    int array[1];
    unsigned long long sz;
    int status;
} r;

// this is hardcoded to divide the work into 10 work chunks
// we should make that a variable later
one_work_t **make_work(int argc, **argv) {
    one_work_t **work_array;
    work_array = (one_work_t *)malloc(10 * sizeof(one_work_t *));
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


int main (int argc, char **argv) {

    struct mw_fxns mw;

    MW_Run (argc, argv, &mw);

    //make_work(1);

    return 0;
}




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