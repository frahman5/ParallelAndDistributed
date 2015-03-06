// *** Structs to hold input and output data *** //
// struct one_work; // Exact definition provided by user //
// struct one_result; // Exact definition provided by user //

struct one_work;
struct one_result;
typedef struct one_work one_work_t;
/* one_work must have two fields with given names
    1) array -- array of fixed size
    2) sz -- unsigned long long that specifies array size
*/

typedef struct one_result one_result_t;
/* one_result must have two fields with given names
    1) array -- array of fixed size
    2) sz -- unsigned long long that specifies array size
    3) status -- int. 0 for success, 1 for failure
*/

// *** Functions to manipulate data *** //
struct mw_fxns {
    // Create the entire work pool. Returns a NULL-terminated list of
    // one_work structs. On failure, return NULL
    one_work_t **(*create_work_pool) (int argc, char **argv); 
    one_result_t *(*do_one_work) (one_work_t* work);
    int (*report_results)(int sz, one_result_t **result_array);
    int work_sz, result_sz;
};

void MW_Run (int argc, char **argv, struct mw_fxns *f);