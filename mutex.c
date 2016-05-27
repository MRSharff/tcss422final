#include <stdlib.h>
#include "mutex.h"
#include "PCB.h"
#include "fifo_queue.h"

Mutex_p Mutex_construct() {
	Mutex_p m = malloc(sizeof(struct Mutex));
	m->owner = NULL;
	m->queue = FIFOq_construct();
	return m;
}

void Mutex_lock(Mutex_p m, PCB_p p) {
	if (m->owner == NULL) {
		m->owner = p;
	} else {
		FIFOq_enqueue(m->queue, p);
	}
}

int Mutex_trylock(Mutex_p m) {
	if (m->owner == NULL) {
		return 0;//lock ok
	} else {
		return 1;//lock failed
	}
}

void Mutex_unlock(Mutex_p m) {
	m->owner = NULL;
}