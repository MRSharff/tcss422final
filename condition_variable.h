#ifndef CONDITION_VARIABLE_H_
#define CONDITION_VARIABLE_H_

#include "mutex.h"

typedef struct thread_lock_node_struct tl_lock_node;
typedef tl_lock_node * tl_lock_node_p;

struct thread_lock_node_struct {
  PCB_p thread;
  Mutex_p lock;
  tl_lock_node_p next;
};

typedef struct tl_queue {
  int size;
  tl_lock_node_p head;
  tl_lock_node_p tail;
} thread_lockq;
typedef thread_lockq * THREAD_LOCKq_p;

typedef struct condition_variable_struct condition_variable;
typedef condition_variable * condition_variable_p;

struct condition_variable_struct {
  unsigned long id;
  int size;
  THREAD_LOCKq_p condition_queue;
};

condition_variable_p condition_variable_construct();

void condition_variable_enqueue(condition_variable_p, PCB_p, Mutex_p);

#endif
