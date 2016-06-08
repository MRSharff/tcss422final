/**
* By Mat Sharff
*/

#ifndef PCB_H_
#define PCB_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "errors.h"

/* Default values */
#define DEFAULT_PID 0
#define DEFAULT_STATE 0
#define DEFAULT_PRIORITY 0
#define DEFAULT_PC 0
#define TRAP_SIZE 4


// /** Exit Codes */
// #define OK 0
// #define NULL_OBJECT 1
// #define FAIL 2

/* Type of PCB*/
#define NORMAL_PCB 0
#define PRODUCER 1
#define CONSUMER 2




typedef enum state_type {
  new, ready, running, interrupted, waiting, halted, terminated
} pcb_state;

typedef struct pcb {
  unsigned long pid; //process ID#, a unique number
  pcb_state state; // process state
  unsigned short priority; // priorities 0=highest, 15=lowest
  unsigned long pc; //holds the current pc value when preempted
  unsigned long sw; // Status word
  unsigned long max_pc; // number of instructions that should be processed before resetting to 0
  clock_t creation; // computer clock when process was created
  clock_t termination; // computer clock when process terminates (goes into list)
  unsigned long terminate; // How many times the PC is allowed hit max_pc
  unsigned long term_count; // How many times the PC HAS hit max_pc

  int io_1_[TRAP_SIZE];
  int io_2_[TRAP_SIZE];

  // New for final project
  unsigned int role; // Type of pcb: 0: normal, 1: producer, 2: consumer
  unsigned long try_lock_trap[TRAP_SIZE];
  unsigned long lock_trap[TRAP_SIZE];
  unsigned long unlock_mutex[TRAP_SIZE];
  unsigned long wait_cond[TRAP_SIZE];
  unsigned long sign_cond[TRAP_SIZE];
  unsigned int *global_variable; //pointer to the global variable.
  unsigned int boost; //if this PCB is low priority, boost it to higher level if this var = 1. after done, change it back to 0.

} PCB;
typedef PCB * PCB_p;

/** Returns pcb pointer to heap allocation */
PCB_p PCB_construct(void);

/** Deallocates pcb from the heap */
void PCB_destruct(PCB_p);

/** Sets the default values for member data */
int PCB_init(PCB_p);

void PCB_randomize_IO_arrays(PCB_p);

/** Sets the PID of the pcb */
int PCB_set_pid(PCB_p, unsigned long);

/** Returns the PID of the pcb */
unsigned long PCB_get_pid(PCB_p);

/** Returns the state of the pcb */
pcb_state PCB_get_state(PCB_p);

/** Sets the state of the pcb */
int PCB_set_state(PCB_p, pcb_state);

/** Returns a string representation of the PCB */
char * PCB_to_string(PCB_p);

/** PCB Test */
int PCB_test(void);

#endif
