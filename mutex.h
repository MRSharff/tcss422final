#ifndef MUTEX_H_
#define MUTEX_H_

#include "fifo_queue.h"

struct Mutex {
	PCB_p owner;
	FIFOq_p queue;
};
typedef struct Mutex * Mutex_p;

Mutex_p Mutex_construct();
void Mutex_lock(Mutex_p, PCB_p);
int Mutex_trylock(Mutex_p);
PCB_p Mutex_unlock(Mutex_p);
#endif
