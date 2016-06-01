#ifndef COND_VAR_TYPE_H
#define COND_VAR_TYPE_H

#include "mutex.h"
#include "fifo_queue.h"

typedef struct cond_var_struct condition_variable;
typedef condition_variable * cond_var_p;

struct cond_var_struct {
	PCB_p thread;
	Mutex_p mutex;
	cond_var_p next;
};

cond_var_p cond_var_p_construct();
void cond_wait(cond_var_p condition_name, Mutex_p mut);
void cond_signal(cond_var_p condition_name);
void cond_var_p_destruct(cond_var_p condition);
#endif
