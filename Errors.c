/*

*/

enum err {NO_ERRORS = 0, NULL_POINTER = 1, INVALID_INPUT = 2};

struct err_desc {
    int  code;
    char *message;
} 

err_desc[] = {
    { NO_ERRORS, "No error." },
    { NULL_POINTER, "Null pointer passed." },
    { INVALID_INPUT, "Invalid Input passed." }
};