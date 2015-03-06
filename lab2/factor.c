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
    int sz;
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
        w->sz = size_of_one_potential_factors_array;

        // put the one_work_t in the work_array
        work_array[work_array_index] = w;

    }

    // Null terminate the list
    work_array[work_array_index] = NULL;

    return work_array;
}

bool it_factors(int multiple, int factor) {
    return (multiple % factor == 0);
}

// do one unit of factoring work
one_result_t *do_work(one_work_t* work)
{
    // Create a one_result_t
    one_result_t *result = (one_result_t *)malloc(sizeof(one_result_t));
    if (!result) {
        printf("We failed to allocate a one_result_t\n");
        exit(1);
    }

    // Create an array into which we can store true factors
    int length = ceil(sqrt(work_sz));
    result->factors = (int *)malloc(length * sizeof(int));
    if (!(result->factors)) {
        printf("We failed to allocate a factors array for a one_result_t\n");
        exit(1);
    }
    int result_factors_index = 0;

    int i;
    for (i = 0; i < length; i++) {
        int factor = work->potential_factors[i];
        if it_factors(work->multiple, factor) {
            if (result_factors_index == length) {
                length *= 2;
                result->factors = realloc(result_factors, length);
                if (!(result->factors)) {
                    printf("We failed to allocate a factors array for a one_result_t\n");
                    exit(1);
                }
            }
            result->factors[result_factors_index++] = factor;
        }
    }

    // Null terminate the list
    result-factors[result_factors_index] = NULL;

    return result;
}


// take an array of results and report the final result
int report(int sz, one_result_t **result_array) {
    printf(" *** Printing results ***\n");
    printf("Factors: ");

    // print the factors
    int i;
    for (i = 0; i < sz; i++) {
        one_result_t *result = result_array[i];
        int j = 1;
        while (j > 0) {
            if (result->factors[j] == NULL) {
                j = -1;
            }
            else {
                printf("%d ", result->factors[j])
            }
        }
    }

    return 0;
}


int main (int argc, char **argv) {


    // Create the api object and give it all its fields
    struct mw_fxns mw;
    mw.create_work_pool = make_work;
    mw.do_one_work = do_work;
    mw.report_results = report;
    mw.work_sz = sizeof(one_work_t);
    mw.result_sz = sizeof(one_result_t);

    //Initialize MPI
    MPI_Init (&argc, &argv);

    // run the program
    MW_Run (argc, argv, &mw);

    MPI_Finalize();
    

    return 0;
}

