#include <mpi.h>    // support for parallel computing
#include <gmp.h>    // support for arbitrarily large integers

#include <stdio.h>  // standard libraries
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "api.h"    // support for master-worker parallelism on top of MPI

#define WORK_ARRAY_SIZE     1000
/* A short program to compute factors using a master-worker paradigm */

// one piece of work to do 
struct one_work {
    mpz_t multiple;
    unsigned long potential_factors[WORK_ARRAY_SIZE];
};

// the result of doing one piece of work
struct one_result {
    unsigned long factors[WORK_ARRAY_SIZE];
};

// set the size in bytes of a one work t, given the multiple we are working with
void get_one_work_size(size_t *size, mpz_t multiple) {
    *size = 500ul;
    // TODO

}
// // divide the factoring work into parallel pieces
one_work_t **make_work(int argc, char **argv) {

    // Check that argv is size 1, and extract the number to be factored
    assert(argc == 2);
    mpz_t multiple;
    mpz_init_set_str(multiple, argv[1], 10);
    gmp_printf("Multiple: %Zd\n", multiple);

    // Calculate the square root
    mpz_t sq_rt_gmp;
    unsigned long sq_rt;
    mpz_init(sq_rt_gmp);
    mpz_sqrt(sq_rt_gmp, multiple);
    sq_rt = mpz_get_ui(sq_rt_gmp);
    sq_rt += 1; // effectively taking the ceiling
    gmp_printf("Sqrt of %Zd is %lu +/- 1\n", multiple, sq_rt);

    // Allocate space for the work item array
    one_work_t **work_array;        
    unsigned long num_work_items;   
    num_work_items = (sq_rt / WORK_ARRAY_SIZE) + 1ul;
    gmp_printf("num work items is: %lu\n", num_work_items);
    work_array = (one_work_t **)malloc((num_work_items + 1)* sizeof(one_work_t *)); // +1 for NULL at the end
    if (!work_array) {
        printf("Failed to allocate space on the heap while creating work array\n");
        exit(1);
    }

    // Allocate and assign the individual work items
    unsigned long work_array_index;
    unsigned long possible_factor = 1;
    for (work_array_index = 0; work_array_index < num_work_items; work_array_index++) {

        // create a one_work_t
        size_t one_work_size;
        get_one_work_size(&one_work_size, multiple);
        one_work_t *w = (one_work_t *)malloc(one_work_size);
        if (!w) {
            printf("Failed to allocate space on the heap while creating one_work_t\n");
            exit(1);
        }

        // fill in the one_work_t with a multiple and factors
        mpz_init_set(w->multiple, multiple);
        int i;
        for (i = 0; i < WORK_ARRAY_SIZE; i++) {
            if (possible_factor <= sq_rt) {
                w->potential_factors[i] = possible_factor++;
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

// int it_factors(int multiple, int factor) {
//     if (factor == 0) {
//         return 0;
//     }
//     return (multiple % factor == 0);
// }

// int isPrime(int factor) {

//     // Special Cases
//     if (factor < 2) {
//         return 0;
//     } else if (factor == 2) {
//         return 1;
//     } else if (factor % 2 == 0) {
//         return 0;
//     }

//     // check for factors
//     int i;
//     int sq_rt = (int)ceil(sqrt(factor));
//     for (i = 3; i <= sq_rt; i += 2) {
//         if (factor % i == 0) {
//             return 0;
//         }
//     }

//     // Otherwise, its prime!
//     return 1;

// } 
// // do one unit of factoring work
one_result_t *do_work(one_work_t* work)
{
    // Create a one_result_t
    one_result_t *result = (one_result_t *)malloc(sizeof(one_result_t));
//     if (!result) {
//         printf("We failed to allocate a one_result_t\n");
//         exit(1);
//     }

//     // Store true factors
//     int i;
//     int result_factors_index = 0;
//     for (i = 0; i < WORK_ARRAY_SIZE; i++) {
//         int factor = work->potential_factors[i];
//         if (it_factors(work->multiple, factor) && isPrime(factor)) {
//             result->factors[result_factors_index++] = factor;
//         }
//     }

//     // Null terminate the list
//     result->factors[result_factors_index] = 0; // when reporting, we can stop scanning the list when we hit a 0

    return result;
}


// // take an array of results and report the final result
int report(int sz, one_result_t **result_array) {
//     printf(" *** Printing results ***\n");
//     printf("Factors: ");

//     // print the factors
//     int i; // iterates through the result_array
//     printf("Value of sz in report function: %d\n", sz);
//     for (i = 0; i < sz; i++) {
//         one_result_t *result = result_array[i];

//         int j = 0; // iterates through a single one_result_t factors arrray
//         while (j > -1) {
//             if (result->factors[j] == 0) {
//                 j = -2; // exit the loop
//             }
//             else {
//                 printf("%d ", result->factors[j++]);
//             }
//         }
//     }
//     printf("\n");

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

// // // Serializes the one_work struct into an array of ints
// // int *(*serialize_one_work)(one_work_t *work) {

// //     // Create the object that we serialize into
// //     int *serial = (int *)malloc((2 + work->sz) * sizeof(int));
// //     if (!serial) {
// //         printf("Failed to allocate while serializing a work struct\n");
// //         exit(1);
// //     }

// //     // Fill it up
// //     serial[0] = work->multiple;
// //     serial[1] = work->sz;
// //     int i;
// //     for (i = 0; i < work->sz; i++) {
// //         serial[i + 2] = work->potential_factors[i];
// //     }

// //     // Return it
// //     return serial;

// // }

// // // Derializes a serialized one_work struct back into a one_work struct
// // one_work_t *(*deserialize_one_work)(int *sz_one_work) {

// //     // Create the one_work_t object
// //     one_work_t *work = (one_work_t *)malloc(sizeof(one_work_t));
// //     if (!work) {
// //         printf("Failed to allocate for a one_work struct space while deserializing a one_work struct\n");
// //         exit(1);
// //     }

// //     // Fill it up
// //     work->multiple = sz_one_work[0];
// //     work->sz = sz_one_work[1];
// //     work->potential_factors = (int *)malloc(work->sz * sizeof(int));
// //     if (!work->potential_factors) {
// //         printf("Failed to allocate space for work->potential_factors while deserializing a one_work_struct\n");
// //         exit(1);
// //     }
// //     int i;
// //     for (i = 0; i < work->sz; i++) {
// //         work->potential_factors[i] = sz_one_work[i+2];
// //     }

// //     // Return it
// //     return work;
// // }
