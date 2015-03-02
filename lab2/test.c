#include <stdio.h>
#include <stdlib.h>
#include "api.h"

struct one_work {
    int *array;
    unsigned long long sz;
} w;

one_work_t **make_work(int num_works) {
    one_work_t **work_array;
    work_array = (one_work_t **)malloc(num_works * sizeof(one_work_t*));
    int i;
    for (i = 0; i < num_works; i++) {
         one_work_t *w = (one_work_t*)malloc(sizeof(one_work_t));
         w->sz = 3+i;
         w->array = (int*)malloc((w->sz)*sizeof(int));
         int j;
         for(j = 0; j < w->sz; ++j)
         {
            w->array[j] = i;
         }
         work_array[i] = w;
        //w->array = array;
        work_array[i] = w;
   }
    // printf("parameter received: %d\n", num_works);

    return work_array;
}
int main (int argc, char **argv) {
    struct mw_fxns mw;

    printf("hello there buddy\n");
    mw.create_work_pool = make_work;
    one_work_t ** test = mw.create_work_pool(4);

    int i = 0;
    for(i = 0; i < 4; ++i)
    {
      //  printf("This is array: %d\n", test[i]->sz);
        printf("This is array: %d\n", i);
        int j = 0;
        for(j = 0; j < test[i]->sz; ++j)
        {
            printf("Array elem %d: %d\n",j, test[i]->array[j]);
        }
    }

    //make_work(1);

    return 0;
}