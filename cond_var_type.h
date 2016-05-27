#ifndef cond_var_type_H
#define cond_var_type_H
#pragma once

#include "mutex.h"
#include "PCB.h"
#include "fifo_queue.h"

struct cond_var_type {
	PCB_p thread;
	Mutex_p mutex;
	struct cond_var_type next;
};
typedef struct cond_var_type * cond_var_p;


cond_var_p cond_var_p_construct();
void cond_wait(cond_var_p condition_name, Mutex_p mut);
void cond_signal(cond_var_p condition_name);
void cond_var_p_destruct(cond_var_p condition);