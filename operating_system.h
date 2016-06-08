// Created by Mat Sharff

#ifndef OPERATING_SYSTEM_H_
#define OPERATING_SYSTEM_H_

#include <time.h>

#include "cond_var_type.h"



#define RUN_TIME 40000
#define CREATE_ITERATIONS 3
#define QUANTUM_DURATION 300
#define MAX_MUTEX_SIZE 10

enum interrupt_type {
  timer, io_1_interrupt, io_2_interrupt, io_1_completion_interrupt, io_2_completion_interrupt, process_termination_interrupt, condition_wait_interrupt
};

/**
 * Returns a PCB_p to be dispatched based on the Round Robin algorithm.
 */
PCB_p round_robin(void);

/**
 * The dispatcher will save the state of the current process into its PCB
 * (here we mean the PC value). It will then dequeue the next waiting process
 * (PCB), change its state to running, and copy its PC value (and SW if you
 * implement it) to the SysStack location to replace the PC of the interrupted
 * process. The dispatcher then returns to the scheduler.
 */
int dispatcher(PCB_p);

int io_service_request_trap_handler(int the_io_device_number);

int process_termination_trap_handler(void);

/**
 * Determines what type of interrupt happened from argument, schedules
 * accordingly, and then calls the dispatcher.
 */
int scheduler(enum interrupt_type);

/**
 * The ISR will change the state of the running process to interrupted, save
 * the CPU state to the PCB for that process (here you need only be concerned
 * with a PC value which, for now, is the integer just described.). And then
 * do an up-call to scheduler.
 */
int pseudo_isr(enum interrupt_type);//, unsigned long *);

/**
 * The cpu is a loop that simulates running of processes. Each loop represents
 * one quantum. It also calls pseudo_isr to simulate a timer interrupt (only
 * interrupt in this version).
 */
void cpu(void);

#endif
