#ifndef PCB_H
#define PCB_H

#include <time.h>

#define MAX_PC_VAL 2500
#define MAX_TERMINATION_COUNT 40
#define IO_TRAPS 4
#define LOCK_TRAP 4
#define TRY_LOCK_TRAP 4
#define UNLOCK_MUTEX_TRAP 4
#define WAIT_COND_TRAP 4 
#define SIGN_COND_TRAP 4 

/* Default values */
#define DEFAULT_PID 0
#define DEFAULT_STATE 0
#define DEFAULT_PC 0
#define DEFAULT_PRIORITY 0

/* Error Handling Values */
#define SUCCESS 0
#define NULL_OBJECT -1

/* Type of PCB*/
#define NORMAL_PCB 0
#define CONSUMER 1
#define PRODUCER 2

/* typedefs for State, PCB, and PCB_p */
typedef enum state_type {
  new, ready, running, interrupted, waiting, halted, terminated
} State;

typedef struct pcb {
  unsigned long pid;
  State state;
  unsigned short priority;
  unsigned long pc;
  unsigned long sw;
  unsigned long max_pc;  // Number of instructions processed by this process before PC returns to 0.
  time_t creation;  // Clock time when this process was created.
  time_t termination;   // Clock time when this process was terminated.
  unsigned int terminate;   // Number of times PC value is reset to 0 before this process terminates.
  unsigned int term_count;  // Number of times PC value has been reset thus far. Used to determine when to terminate the process.
  unsigned int role; // Type of pcb: 0: normal, 1: consumer or  2: producer.
  unsigned long io_1_traps[IO_TRAPS];   // Array of PC values when process executes an I/O 1 service trap.
  unsigned long io_2_traps[IO_TRAPS];   // Array of PC values when process executes an I/O 2 service trap.
  unsigned long lock_trap[LOCK_TRAP];
  unsigned long try_lock_trap[TRY_LOCK_TRAP];
  unsigned long unlock_mutex[UNLOCK_MUTEX_TRAP];
  unsigned long wait_cond[WAIT_COND_TRAP];
  unsigned long sign_cond[SIGN_COND_TRAP];
  unsigned int *glo_var;// pointer to the global variable.
  unsigned int boost;//if this PCB is low priority, boost it to higher level if this var = 1. after done, change it back to 0.
} PCB;

typedef PCB * PCB_p;

/* Function Prototypes */
PCB_p PCB_construct(void);
void PCB_destruct(PCB_p pcb);
int PCB_init(PCB_p pcb);
char * PCB_toString(PCB_p pcb);

#endif
