#ifndef ERRORS_H_
#define ERRORS_H_

/*
	Helper class that includes enum of errors and a struct containing error code and message, used for PCB.c
	
	@version 1
	4/8/16
	@author Joshua Cho
*/


enum err {
	NO_ERRORS, // 0
	NULL_POINTER, // 1
	INVALID_INPUT // 2
};


// enum err {NO_ERRORS = 0, NULL_POINTER = 1, INVALID_INPUT = 2};

// struct err_desc {
//     int  code;
//     char *message;
// } 

// err_desc[] = {
//     { NO_ERRORS, "No error." },
//     { NULL_POINTER, "Null pointer passed." },
//     { INVALID_INPUT, "Invalid Input passed." }
// };
 

 void print_error(int);
// void print_error(int err) {	//prints error message
// 	printf("%s", error_names[err]);
// }

#endif