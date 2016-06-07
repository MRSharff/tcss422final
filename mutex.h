#ifndef MUTEX_H_
#define MUTEX_H_

#include "priority_queue.h"

struct Mutex {
	PCB_p owner;
	FIFOq_p queue;
};
typedef struct Mutex * Mutex_p;

Mutex_p Mutex_construct();
void Mutex_lock(Mutex_p m, PCB_p p);
int Mutex_trylock(Mutex_p m);
void Mutex_unlock(Mutex_p m, PRIORITYq_p);
#endif
