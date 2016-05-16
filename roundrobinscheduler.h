#ifndef ROUND_ROBIN_SCHEDULER_H_
#define ROUND_ROBIN_SCHEDULER_H_

#include "queue.h"

#define DEFAULT_NUM_PCBS 20
#define QUANTUM 1

enum interrupt_type {timer, IO, interrupt};	

typedef struct cpu {
	unsigned int pcRegister;
	void(*mainLoop) (struct cpu *);
}CPU;

int CheckIOTrap1(PCB_p);
int CheckIOTrap2(PCB_p);
PCB_p IOTrapHandler(int, Queue_q, Queue_q, PCB_p) ;
void mainloopFunction(struct cpu *);
int randomNumber (int, int); 
void NewToReady(Queue_q, Queue_q);
PCB_p ISR(enum interrupt_type, PCB_p, Queue_q);
PCB_p scheduler(enum interrupt_type, PCB_p, Queue_q);
PCB_p dispatcher(PCB_p, Queue_q);
PCB_p RoundRobinPrint(PCB_p, Queue_q);
void* Timer();
void* io_trap1();
void* io_trap2();

#endif