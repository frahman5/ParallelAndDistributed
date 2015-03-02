#include <stdio.h>
#include <stdlib.h>
#include "api.h"

struct one_work {
    int *array;
    unsigned long long sz;
} w;

one_work_t **make_work(int num_works) {
    one_work_t **work_array;
    *work_array = (one_work_t *)malloc(num_works * sizeof(one_work_t));
    // int i;
    // for (i = 0; i < num_works; i++) {
    //     one_work_t *w;
    //     w->sz = 3;
    //     int *array = (int *)malloc(w->sz * sizeof(int));
    //     array[0] = 1;
    //     array[1] = 2;
    //     array[2] = 3;
    //     w->array = array;
    //     *work_array[i] = *w;
    // }
    // printf("parameter received: %d\n", num_works);
    printf("HH\n");

    return work_array;
}
int main (int argc, char **argv) {
    struct mw_fxns mw;

    printf("hello there buddy\n");
    mw.create_work_pool = make_work;
    mw.create_work_pool(4);
    //make_work(1);

    return 0;
}