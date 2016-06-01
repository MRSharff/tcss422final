#include <stdlib.h>
#include "cond_var_type.h"

cond_var_p cond_var_p_construct(){
	cond_var_p temp = (cond_var_p)malloc(sizeof(struct cond_var_struct));
	temp->thread = NULL;
	temp->mutex=NULL;
	temp->next = NULL;
}

void cond_wait(cond_var_p condition_name, Mutex_p the_mutex){
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
		Mutex_unlock(the_mutex);
	}
}

void cond_signal(cond_var_p condition_name){
	FIFOq_enqueue(condition_name->mutex->queue,condition_name->thread);
	cond_var_p temp = condition_name;
	condition_name = condition_name->next;
	cond_var_p_destruct(temp);
}

void cond_var_p_destruct(cond_var_p condition){
	condition-> next = NULL;
	condition->thread = NULL;
	condition->mutex = NULL;
	free(condition);
}
