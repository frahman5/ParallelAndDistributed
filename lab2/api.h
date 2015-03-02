// *** Structs to hold input and output data *** //
// struct one_work; // Exact definition provided by user //
// struct one_result; // Exact definition provided by user //
typedef struct one_work one_work_t;
typedef struct one_result one_result_t;

/* one_work must have two fields with given names
    1) array -- array of fixed size
    2) sz -- unsigned long long that specifies array size
*/

/* one_result must have two fields with given names
    1) array -- array of fixed size
    2) sz -- unsigned long long that specifies array size
    3) status -- int. 0 for success, 1 for failure
*/

// *** Functions to manipulate data *** //
struct mw_fxns {
    // Create the entire work pool. Returns a NULL-terminated list of
    // one_work structs. On failure, return NULL
    one_work_t **(*create_work_pool) (int dummy_input); 
};