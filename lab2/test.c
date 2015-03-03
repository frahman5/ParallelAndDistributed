#include <stdio.h>
#include <stdlib.h>
#include "api.h"

struct one_work {
    int *array;
    unsigned long long sz;
} w;

one_work_t **make_work(int argc, char **argv) {
    return NULL;
}

struct one_result {
    int *array;
    unsigned long long sz;
} r;


one_result_t *do_work(one_work_t* work)
{
    return NULL;
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