#include <stdlib.h>
#include "cond_var_type.h"


cond_var_p cond_var_p_construct(){
	cond_var_p temp = (cond_var_p)malloc(sizeof(struct cond_var_struct));
	temp->thread = NULL;
	temp->mutex = NULL;
	temp->next = NULL;
}

void cond_wait(cond_var_p condition_name, Mutex_p the_mutex, PRIORITYq_p the_ready_queue){
	if(condition_name != NULL && the_mutex != NULL){
		cond_var_p temp = condition_name;
		cond_var_p temp1 = temp->next;
		while(temp1 != NULL){
			temp1 = temp1->next;
			temp = temp-> next;
		}
		cond_var_p temp2 = (cond_var_p) malloc(sizeof(struct cond_var_struct));
		temp2->thread = the_mutex->owner;
		temp2->mutex = the_mutex;
		temp2->next = NULL;
		temp->next = temp2;
		Mutex_unlock(the_mutex, the_ready_queue);
	}
}

PCB_p cond_signal(cond_var_p condition_name){
  if (condition_name != NULL) {
    PCB_p temp_pcb = condition_name->thread;
  	Mutex_p temp_mutex = condition_name->mutex;
  	FIFOq_enqueue(condition_name->mutex->queue,condition_name->thread);
  	cond_var_p temp = condition_name;
  	condition_name = condition_name->next;
  	Mutex_lock(temp_mutex,temp_pcb);
  	cond_var_p_destruct(temp);
  	return temp_pcb;
  }
  print_error(NULL_POINTER);
  printf("in cond_signal, condition_name was null");
  return NULL;
}

void cond_var_p_destruct(cond_var_p condition){
	condition-> next = NULL;
	condition->thread = NULL;
	condition->mutex = NULL;
	free(condition);
}
