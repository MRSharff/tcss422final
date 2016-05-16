#include <stdio.h>
#include "errors.h"

// enum err {NO_ERRORS = 0, NULL_POINTER = 1, INVALID_INPUT = 2};

char * error_names[3] = {
	"No error.",
	"Null pointer passed.",
	"Invalid Input passed."
};


// struct err_desc {
//     int  code;
//     char *message;
// } 

// err_desc[] = {
//     { NO_ERRORS, "No error." },
//     { NULL_POINTER, "Null pointer passed." },
//     { INVALID_INPUT, "Invalid Input passed." }
// };

void print_error(int error_num) {	//prints error message
	printf("%s", error_names[error_num]);
}