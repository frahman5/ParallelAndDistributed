#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <assert.h>
#include "api.h"


/* A short program to compute factors using a master-worker paradigm */

// one piece of work to do 
struct one_work {
    int multiple;
    int *potential_factors;
};

// the result of doing one piece of work
struct one_result {
    int *factors;
};

// divide the factoring work into parallel pieces
one_work_t **make_work(int argc, char **argv) {

    // Check that argv is size 1, and extract the number to be factored
    assert(argc == 2);
    int multiple = atoi(argv[1]);

    // Calculate the square root
    int sq_rt = (int) ceil(sqrt(multiple));
    printf("the square root of %d is %d\n", multiple, sq_rt);

    // Create work items
    one_work_t **work_array;
    int size_of_one_potential_factors_array = (int) ceil(sqrt(sq_rt));
    int num_work_items = ceil(sq_rt/(size_of_one_potential_factors_array); // e.g if multiple = 101, 11 / 4 = 3
    work_array = (one_work_t **)malloc((num_work_items + 1)* sizeof(one_work_t *)); // +1 for NULL at the end

    int work_array_index;
    int possible_factor = 1;
    for (work_array_index = 0; work_array_index < num_work_items; work_array_index++) {

        // create a one_work_t
        one_work_t *w = (one_work_t *)malloc(sizeof(one_work_t));

        // fill in the one_work_t with a multiple and fators
        w->multiple = multiple;
        w->potential_factors = (int *)malloc(size_of_one_potential_factors_array * sizeof(int));
        int i;
        for (i = 0; i < size_of_one_potential_factors_array; i++) {
          if (potential_factor <= sq_rt) {
                  w->potential_factors[i] = potential_factor++;
            }
            else {
                  w->potential_factors[i] = 0;
            }
        }

        // put the one_work_t in the work_array
        work_array[work_array_index] = w;

    }

    // Null terminate the list
    work_array[work_array_index] = NULL;

    return work_array;
}

// // do one unit of factoring work
// one_result_t *do_work(one_work_t* work)
// {
//     // calculate the result
//     int answer = 0;
//     int i = 0;
//     for (i = 0; i < WORK_ARRAY_SIZE; i++) {
//         answer += work->array[i];
//     }

//     // pack the result up and return it
//     one_result_t *result = (one_result_t *)malloc(sizeof(one_result_t));
//     result->array[0] = answer;
//     result->status = 0;

//     return result;
// }


// // take an array of results and report the final result
// int report(int sz, one_result_t **result_array) {
//     int i;
//     int result = 0;
//     for (i = 0; i < sz; i++) {
//         result += result_array[i]->array[0];
//     }

//     printf("Result of computation: %d\n", result);

//     return 0;
// }


int main (int argc, char **argv) {


    make_work(argc, argv);
    // // Create the api object and give it all its fields
    // struct mw_fxns mw;
    // mw.create_work_pool = make_work;
    // mw.do_one_work = do_work;
    // mw.report_results = report;
    // mw.work_sz = sizeof(one_work_t);
    // mw.result_sz = sizeof(one_result_t);

    // //Initialize MPI
    // MPI_Init (&argc, &argv);

    // // run the program
    // MW_Run (argc, argv, &mw);

    // MPI_Finalize();
    

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