#include <stdlib.h>
#include "mutex.h"

Mutex_p Mutex_construct() {
	Mutex_p m = malloc(sizeof(struct Mutex));
	m->owner = NULL;
	m->queue = FIFOq_construct();
  FIFOq_init(m->queue);
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
		return 0; //lock ok
	} else {
		return 1; //lock failed
	}
}

void Mutex_unlock(Mutex_p m) {

  if (m->owner == NULL) { // only for debugging
    printf("Unlocked called before any lock\n");
  }

  if (m != NULL) {
    m->owner = FIFOq_dequeue(m->queue);
  } else {
    printf("mutex was null in Mutex_unlock\n");
  }

  //return m->owner; // Need to enqueue back into ready queue
  //TODO: In OS loop, set the returned process state to ready and enqueue in ready queue
}
