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

    /* For serialization functions, if your struct requires no serialization (
    for example, there are no pointers inside your struct), then simply write a 
    function that does nothing and set serialize_work and/or serialize_result to false */

    // Serializes the one_work struct into an array of ints
    int *(*serialize_one_work)(one_work_t *work);

    // Derializes a serialized one_work struct back into a one_work struct
    one_work_t *(*deserialize_one_work)(int *sz_one_work);

    // Serializes the one_result struct into an array of ints
    int *(*serialize_one_result)(one_result_t *result);

    // Deserializes a serialized one_result struct back into a one_result struct
    one_result_t *(*deserialize_one_result)(int *sz_one_result);

    /* Specify the size in bytes of one_work and one_result */
    int work_sz, result_sz;

    /* Whether or not your one_work and one_result functions need serialization */
    bool serialize_work, serialize_result;
};

void MW_Run (int argc, char **argv, struct mw_fxns *f);