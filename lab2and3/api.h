// *** Structs to hold input and output data *** //
// struct one_work; // Exact definition provided by user //
// struct one_result; // Exact definition provided by user //

struct one_work;
struct one_result;
typedef struct one_work one_work_t;
typedef struct one_result one_result_t;

// *** Functions to manipulate data *** //
struct mw_fxns {

    /* Creates the entire work, returning a NULL-terminated list of
    one_work structs. On failure, returns NULL */
    one_work_t **(*create_work_pool) (int argc, char **argv); 

    /* Computes the results of one piece of work, and returns it 
    in a pointer to a one_result struct */
    one_result_t *(*do_one_work) (one_work_t* work);

    /* Takes an array of pointers to one_results, and a size of that array. 
    Prints the results to file or stdout, as per the user's desire. 
    Returns 0 on failure, 1 on success */
    int (*report_results)(int sz, one_result_t **result_array);

    // bogus debugging function
    one_result_t *(*initializeResult) (one_result_t *result);

    /* Specify the size in bytes of one_work and one_result */
    int work_sz, result_sz;

};

/* If style is 1, allocates work via a round robbin method. If style is 2, 
allocates work dynamically, giving each worker more work as they finish */
void MW_Run (int argc, char **argv, struct mw_fxns *f, int style);
// void MW_Run_2 (int argc, char **argv, struct mw_fxns *f);