#include <stdlib.h>
#include "cond_var_type.h"


cond_var_p cond_var_p_construct(){
	cond_var_p temp = calloc(1, sizeof(condition_variable));
	temp->next_thread = NULL;
  temp->next_lock = NULL;
  return temp;
}

// in OS, we need to deschedule the current process (the_calling_thread) and call scheduler
void cond_wait(cond_var_p condition_var, Mutex_p the_lock, PCB_p the_calling_thread, PRIORITYq_p the_ready_queue){
	if(condition_var != NULL && the_lock != NULL && the_calling_thread != NULL){
		Mutex_unlock(the_lock, the_ready_queue);
    condition_var->next_thread = the_calling_thread;
    condition_var->next_lock = the_lock;
    printf("PID %lu requested condition wait on cond %p with mutex %p\n", the_calling_thread->pid, &condition_var, &the_lock);
	} else {
    printf("In cond_wait, a variable is null\n");
  }
}

PCB_p cond_signal(cond_var_p condition_var){
  if (condition_var != NULL && condition_var->next_thread != NULL) {
    PCB_p temp_thread = condition_var->next_thread;
    Mutex_p temp_lock = condition_var->next_lock;
    PCB_set_state(temp_thread, ready);

    // reacquire lock
    Mutex_lock(temp_lock, temp_thread);
    condition_var->next_thread = NULL;
    condition_var->next_lock = NULL;
    return temp_thread;
  }
  // print_error(NULL_POINTER);
  // printf("in cond_signal, condition_var was null\n");
  return NULL;
}

void cond_var_p_destruct(cond_var_p condition){
	condition->next_thread = NULL;
	condition->next_lock = NULL;
	free(condition);
}
