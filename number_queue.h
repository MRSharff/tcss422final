/**
* By Mat Sharff
*/

#ifndef NUMBER_QUEUE_H_
#define NUMBER_QUEUE_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

typedef struct number_node_struct number_node;
typedef number_node * number_node_p;

struct number_node_struct {
  int available_index;
  number_node_p next;
};

typedef struct NUMBER_q {
  int size;
  number_node_p head;
  number_node_p tail;
} NUMBERq;
typedef NUMBERq * NUMBERq_p;

/** Returns NUMBERq pointer to heap allocation. */
NUMBERq_p NUMBERq_construct(void);

/** Sets the default values for NUMBERq member data. */
int NUMBERq_init(NUMBERq_p, int);

/** Enqueues a int in the queue. */
int NUMBERq_enqueue(NUMBERq_p, int);

/** Returns the int at the head of the queue and dequeues it. */
int NUMBERq_dequeue(NUMBERq_p);

/** Returns the int at the head of the queue but does not dequeue it. */
int NUMBERq_peek(NUMBERq_p);

/** Deallocates NUMBERq from the heap. */
int NUMBERq_destruct(NUMBERq_p);

/** Returns 1 if the NUMBERq is empty, 0 otherwise. */
int NUMBERq_is_empty(NUMBERq_p);

/** Returns the size of the NUMBERq. */
int NUMBERq_size(NUMBERq_p);

// // /** Returns a string representation of the NUMBERq. */
// char * NUMBERq_to_string(NUMBERq_p);
//
// /** Test method */
// int NUMBERq_test(int);

#endif
