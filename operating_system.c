// Created by Mat Sharff

#include "operating_system.h"

#define MAX_LOOPS 100000

// producer consumer program
#define FIRST_MUTEX_LOCK_INSTRUCTION 301
#define WAIT_COND_INSTRUCTION 303
#define FIRST_MUTEX_UNLOCK_INSTRUCTION 304
#define INCREMENT_INSTRUCTION 450
#define PRINT_INSTRUCTION 450
#define SECOND_MUTEX_LOCK_INSTRUCTION 655
#define CHANGE_INCREMENT_AVAIL_INSTRUCTION 656
#define COND_SIGNAL_INSTRUCTION 657
#define SECOND_MUTEX_UNLOCK_INSTRUCTION 658

// mutual resource program
#define LOCK_R1_INSTRUCTION 1
#define LOCK_R2_INSTRUCTION 500
#define PRINT_TO_TRACE_INSTRUCTION 501
#define UNLOCK_R2_INSTRUCTION 900
#define UNLOCK_R1_INSTRUCTION 905


// deadlock program
#define DEADLOCK_R2_INSTRUCTION 300
#define DEADLOCK_R1_INSTRUCTION 301
#define PRINT_TO_TRACE_INSTRUCTION2 302
#define DEADUNLOCK_R1_INSTRUCTION 303
#define DEADUNLOCK_R2_INSTRUCTION 304


// Trackers for amount of processes
int io_process_count;
int compute_intensive_processes;

//The following are initialized in main
unsigned long pid_counter; // Needs to be mutex locked if using multiple cpus on different threads
unsigned long pc_register;
unsigned long sys_stack;
int dispatch_counter;
int create_count;
int io_1_downcounter;
int io_2_downcounter;
int timer_count;

//Producer increment instruction value (this is essentially the pseudo program)

const unsigned long first_mutex_lock_instruction = 301; // thread_mutex_lock(mutex[x])
const unsigned long wait_cond_instruction = 303; // if (increment_avail == 0 (or 1 for consumer)) thread_cond_wait(num_not_incremented (for consumer, num_not_printed), mutex[x])
const unsigned long first_mutex_unlock_instruction = 304; // thread_mutex_unlock(mutex[x])
const unsigned long increment_instruction = 450;
const unsigned long print_instruction = 450;
const unsigned long second_mutex_lock_instruction = 655;
const unsigned long change_increment_avail_instruction = 656;
const unsigned long cond_signal_instruction = 657;
const unsigned long second_mutex_unlock_instruction = 658;


// Producer Consumer Pairs (hard coded 5 pairs)
PCB_p producer[5];
PCB_p consumer[5];


Mutex_p mutex[5];

PCB_p mutual_resource_1[1];
PCB_p mutual_resource_2[1];

Mutex_p r1_mutual_resource_mutex[1];
Mutex_p r2_mutual_resource_mutex[1];
// analogous to bufavail, 0 to 1
int increment_avail[5];

// condition variables
//like cond_var_type buf_not_full, used by producer
cond_var_p num_not_incremented[5];

// like cond_var_type buf_not_empty, used by consumer
cond_var_p num_not_printed[5];

// pointed to by global variable in pcb
int prod_cons_shared_var[5];


//TODO: you must also alter your random generating algorithm for
//the I/O arrays to make sure none of the numbers recorded there
//interfere with these hand-crafted numbers, i.e. you don't want
//to trap to an I/O after locking a mutex!



// Producer-Consumer Pair mutexes // not used anymore because we hard coded Prod-cons amount
// Mutex_p producer_consumer_mutexes[MAX_MUTEX_SIZE]; // no more than 10
// NUMBERq_p available_index_queue;

int total_processes_created;

PCB_p idl;
FIFOq_p created_queue;
PRIORITYq_p ready_queue; // Could use Priority queue since all PCB priorities will be same value
FIFOq_p io_1_waiting_queue;
FIFOq_p io_2_waiting_queue;
FIFOq_p terminate_queue;

//This pointer is changing constantly, initially set to point to idl in main
PCB_p current_process;

/*************************************************************
*     Helper methods
**************************************************************/

long true_random(long max) {
  unsigned long num_bins, num_rand, bin_size, defect;
  long x;
  num_bins = (unsigned long) max + 1;
  num_rand = (unsigned long) RAND_MAX + 1;
  bin_size = num_rand / num_bins;
  defect = num_rand % num_bins;
  do {
    x = random();
  } while (num_rand - defect <= (unsigned long)x);
  return x/bin_size;
}

// Final Project Create processes section
//*************************************************************************

// Create a set of processes that do not request I/O or synchronization services.
// These are just fillers to take up time. Dynamically create and terminate with
// no more than 25 running at a time.
int create_compute_intensive_processes(int amount) {
  PCB_p temp;
  int i;
  for (i = 0; i < amount; i++) {
    temp = PCB_construct();
    PCB_init(temp);
    temp->max_pc = (rand() % 4000) + 2000;
    temp->terminate = rand() % 5 + 1; // These compute intensive processes do not terminate
    temp->term_count = 0;
    PCB_set_pid(temp, pid_counter);
    pid_counter++;
    FIFOq_enqueue(created_queue, temp);
    total_processes_created++;
  }
  return 0; // for right now
}

int deadlock_check() {
  int i;
  Mutex_p temp1, temp2;
  for (i = 0; i < 1; i++) {
    temp1 = r1_mutual_resource_mutex[i];
    temp2 = r2_mutual_resource_mutex[i];
    if ((temp1 != NULL && temp1->owner != NULL && !FIFOq_is_empty(temp1->queue))
          && (temp2 != NULL && temp2->owner != NULL && !FIFOq_is_empty(temp2->queue))) {
      printf("deadlock detected for processes PID%lu and PID12\n", temp1->owner->pid);
      return 1;
    }
  }
  printf("No deadlocks were detected\n");
  return 0;
}

int create_mutual_resource_pairs() {
  int i, j, k;
  unsigned long var_val;
  unsigned int pair_max_pc = 1000;
  for (i = 0; i < 1; i++) { // 5 pairs
    var_val = 300;
  	mutual_resource_1[i] = PCB_construct();
  	mutual_resource_2[i] = PCB_construct();
  	PCB_init(mutual_resource_1[i]);
  	PCB_init(mutual_resource_2[i]);
  	mutual_resource_1[i]->priority = 1;
  	mutual_resource_2[i]->priority = 1;
  	mutual_resource_1[i]->role = MUTUAL_RESOURCE_USER;
  	mutual_resource_2[i]->role = MUTUAL_RESOURCE_USER;

  	r1_mutual_resource_mutex[i] = Mutex_construct();
    r2_mutual_resource_mutex[i] = Mutex_construct();

  	PCB_set_pid(mutual_resource_1[i], pid_counter);
    pid_counter++;
  	PCB_set_pid(mutual_resource_2[i], pid_counter);
    pid_counter++;

  	mutual_resource_1[i]->max_pc = pair_max_pc;
  	mutual_resource_2[i]->max_pc = pair_max_pc;



  	PRIORITYq_enqueue(ready_queue, mutual_resource_1[i]);
  	PRIORITYq_enqueue(ready_queue, mutual_resource_2[i]);
    total_processes_created++;
    total_processes_created++;
  }
  mutual_resource_1[0]->lock_trap[0] = LOCK_R1_INSTRUCTION;
  mutual_resource_1[0]->lock_trap[1] = LOCK_R2_INSTRUCTION;
  mutual_resource_1[0]->unlock_mutex[0] = UNLOCK_R1_INSTRUCTION;
  mutual_resource_1[0]->unlock_mutex[1] = UNLOCK_R2_INSTRUCTION;


  mutual_resource_2[0]->lock_trap[0] = DEADLOCK_R2_INSTRUCTION;
  mutual_resource_2[0]->lock_trap[1] = DEADLOCK_R1_INSTRUCTION;
  mutual_resource_2[0]->unlock_mutex[0] = DEADUNLOCK_R1_INSTRUCTION;
  mutual_resource_2[0]->unlock_mutex[1] = DEADUNLOCK_R2_INSTRUCTION;

}




int create_producer_consumer_pairs() {
  int i, j, k;
  unsigned long var_val;
  unsigned int pair_max_pc = 1000;
  for (i = 0; i < 5; i++) { // 5 pairs
    var_val = 300;
  	producer[i] = PCB_construct();
  	consumer[i] = PCB_construct();
  	PCB_init(producer[i]);
  	PCB_init(consumer[i]);
  	producer[i]->priority = 1;
  	consumer[i]->priority = 1;
  	producer[i]->role = PRODUCER;
  	consumer[i]->role = CONSUMER;

  	increment_avail[i] = 1;

  	mutex[i] = Mutex_construct();

  	PCB_set_pid(producer[i], pid_counter);
    pid_counter++;
  	PCB_set_pid(consumer[i], pid_counter);
    pid_counter++;

    num_not_incremented[i] = cond_var_p_construct();
    num_not_printed[i] = cond_var_p_construct();

  	prod_cons_shared_var[i]=0;
  	producer[i]->global_variable = &prod_cons_shared_var[i];
  	consumer[i]->global_variable = &prod_cons_shared_var[i];
  	producer[i]->max_pc = pair_max_pc;
  	consumer[i]->max_pc = pair_max_pc;

    producer[i]->try_lock_trap[0] = 300; // 300
    consumer[i]->try_lock_trap[0] = 300; // 300
    producer[i]->lock_trap[0] = FIRST_MUTEX_LOCK_INSTRUCTION; // 301
    consumer[i]->lock_trap[0] = FIRST_MUTEX_LOCK_INSTRUCTION; // 301
    producer[i]->wait_cond[0] = WAIT_COND_INSTRUCTION; // 303
    consumer[i]->wait_cond[0] = WAIT_COND_INSTRUCTION; // 303
    producer[i]->unlock_mutex[0] = FIRST_MUTEX_UNLOCK_INSTRUCTION; // 304
    consumer[i]->unlock_mutex[0] = FIRST_MUTEX_UNLOCK_INSTRUCTION; // 304
    producer[i]->try_lock_trap[1] = 654; // 654
    consumer[i]->try_lock_trap[1] = 654; // 654
    producer[i]->lock_trap[1] = SECOND_MUTEX_LOCK_INSTRUCTION; // 655
    consumer[i]->lock_trap[1] = SECOND_MUTEX_LOCK_INSTRUCTION; // 655
    producer[i]->sign_cond[1] = COND_SIGNAL_INSTRUCTION; // 657
    consumer[i]->sign_cond[1] = COND_SIGNAL_INSTRUCTION; // 657
    producer[i]->unlock_mutex[1] = SECOND_MUTEX_UNLOCK_INSTRUCTION; // 658
    consumer[i]->unlock_mutex[1] = SECOND_MUTEX_UNLOCK_INSTRUCTION; // 658

      // producer[i]->try_lock_trap[0] = var_val; // 300
      // consumer[i]->try_lock_trap[0] = var_val; // 300
      // var_val+= 1;
      // producer[i]->lock_trap[0] = var_val; // 301
      // consumer[i]->lock_trap[0] = var_val; // 301
      // var_val+= 2;
      // producer[i]->wait_cond[0] = var_val; // 303
      // consumer[i]->wait_cond[0] = var_val; // 303
      // var_val+= 1;
      // producer[i]->unlock_mutex[0] = var_val; // 304
      // consumer[i]->unlock_mutex[0] = var_val; // 304
      // var_val+= 350;
      // producer[i]->try_lock_trap[1] = var_val; // 654
      // consumer[i]->try_lock_trap[1] = var_val; // 654
      // var_val+= 1;
      // producer[i]->lock_trap[0] = var_val; // 655
      // consumer[i]->lock_trap[0] = var_val; // 655
      // var_val+= 2; // changing increment_avail happens at 656
      // producer[i]->sign_cond[1] = var_val; // 657
      // consumer[i]->sign_cond[1] = var_val; // 657
      // var_val+= 1;
      // producer[i]->unlock_mutex[1] = var_val; // 658
      // consumer[i]->unlock_mutex[1] = var_val; // 658
  	PRIORITYq_enqueue(ready_queue, producer[i]);
  	PRIORITYq_enqueue(ready_queue, consumer[i]);
    total_processes_created++;
    total_processes_created++;
  }
}

//*************************************************************************
// End final project Create processes section



PCB_p round_robin() {
  if (ready_queue != NULL) {
    if (!PRIORITYq_is_empty(ready_queue)) {
      return PRIORITYq_dequeue(ready_queue);
    }
    return idl; // no processes in ready queue, idl should be put onto ready queue
  }
  printf("in round_robin: ready_queue is null");
  return NULL;
}

// Puts all pcbs stuff into cpu registers
int dispatcher(PCB_p pcb_to_dispatch){
  char * string; //free this after printing

  if (pcb_to_dispatch != NULL) { // error checking
    if (dispatch_counter == 4) { // really only for printing purposes
      dispatch_counter = 0;
      printf("Switching to: %s\n", PCB_to_string(pcb_to_dispatch));
      PCB_set_state(pcb_to_dispatch, running); // this is done here for printing, but can be deleted

      printf("Now running: %s\n", PCB_to_string(pcb_to_dispatch));
      printf("Returned to Ready Queue: %s\n", PCB_to_string(current_process));

      string = PRIORITYq_to_string(ready_queue);
      printf("Ready Queue: %s\n\n", string);
      free(string);
    }
    PCB_set_state(pcb_to_dispatch, running); // set state to running
    sys_stack = pcb_to_dispatch->pc; //
    // pc_register = pcb_to_dispatch->pc; // put pc into cpu register
    current_process = pcb_to_dispatch; // dispatch the process
    dispatch_counter++; // increment for dispatch printing
    return NO_ERRORS;
  }

  printf("in dispatcher: pcb_to_dispatch was null\n");
  exit(NULL_POINTER);
  return NULL_POINTER;
}

int timer_interrupt_handler(void) {
  if (current_process != NULL) {
  // if (1) {
    current_process->pc = sys_stack;//pc_register; // Save context in PCB
    PCB_set_state(current_process, ready); // Set state to ready because it was just a timer interrupt
    if (current_process->pid != idl->pid) {
      //only do this if the current_process is not the idle process
      // we don't want to enqueue idl into ready queue
      if (dispatch_counter != 4) {
        printf("Process enqueued: %s\n", PCB_to_string(current_process));
      }
      PRIORITYq_enqueue(ready_queue, current_process);
    }
    // if current_process was the idl process, we can just replace it
    return NO_ERRORS;
  }
  printf("in timer_interrupt_handler: current_process was null\n");
  exit(NULL_POINTER);
  return NULL_POINTER;
}

int io_request_trap(int the_io_device_number) {
  if (current_process != NULL) {
    current_process->pc = sys_stack;//pc_register; // Save context in PCB
    PCB_set_state(current_process, waiting);
    if (the_io_device_number == io_1_interrupt) {
      FIFOq_enqueue(io_1_waiting_queue, current_process); // move current process to waiting queue
      if (io_1_downcounter == -1) { // if the io is not currently counting down, start it
        // we will also restart the downcounter when io completes and still has stuff in the waiting queue
        io_1_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // multiply quantum 3-5 times
      }
    } else {
      FIFOq_enqueue(io_2_waiting_queue, current_process); // move current process to waiting queue
      if (io_2_downcounter == -1) { // if the io is not currently counting down, start it
        // we will also restart the downcounter when io completes and still has stuff in the waiting queue
        io_2_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // multiply quantum 3-5 times
      }
    }
    return NO_ERRORS;
  }
  printf("in io_request_trap: current_process was null\n");
  exit(NULL_POINTER);
  return NULL_POINTER;
}

int io_completion_interrupt_handler(int the_io_device_number) {
  PCB_p temp;
  if (current_process != NULL) {
    current_process->pc = sys_stack;//pc_register; // Save context in PCB
    if (the_io_device_number == io_1_completion_interrupt) {
      temp = FIFOq_dequeue(io_1_waiting_queue); // Dequeue process from io queue
      PCB_set_state(temp, ready); // Set state to ready
      if (!FIFOq_is_empty(io_1_waiting_queue)) { // if the queue still has stuff in it, reset the counter
        io_1_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // multiply quantum 3-5 times
      }
    } else {
      temp = FIFOq_dequeue(io_2_waiting_queue); // Dequeue process from io queue
      PCB_set_state(temp, ready); // Set state to ready
      if (!FIFOq_is_empty(io_2_waiting_queue)) { // if the queue still has stuff in it, reset the counter
        io_2_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // multiply quantum 3-5 times
      }
    }
    if (temp != NULL) {
      PRIORITYq_enqueue(ready_queue, temp); // move current process ready queue
      return NO_ERRORS;
    } else {
      printf("in io_request_trap: temp was null\n");
      exit(NULL_POINTER);
      return NULL_POINTER;
    }
  }
  printf("in io_completion_interrupt_handler: current_process was null\n");
  exit(NULL_POINTER);
  return NULL_POINTER;
}

int process_termination_trap_handler(void) {
  if (current_process != NULL) {
    current_process->pc = sys_stack;//pc_register; // Save context in PCB
    PCB_set_state(current_process, halted);
    current_process->termination = clock();
    FIFOq_enqueue(terminate_queue, current_process);
    return NO_ERRORS;
  }
  printf("in process_termination_trap_handler: current_process was null\n");
  exit(NULL_POINTER);
  return NULL_POINTER;
}

int condition_wait_handler(void) {
  if (current_process != NULL) {
    current_process->pc = sys_stack;//pc_register; // Save context in PCB
    PCB_set_state(current_process, waiting);
    current_process = NULL;
    return NO_ERRORS;
  }
  printf("in process_termination_trap_handler: current_process was null\n");
  exit(NULL_POINTER);
  return NULL_POINTER;
}

int scheduler(enum interrupt_type int_type) {
  int return_status;
  PCB_p pcb_to_be_dispatched; // determined by scheduler algorithm RR, SJF, PRIORTY, etc
  PCB_p temp;

  while (!FIFOq_is_empty(created_queue)) {
    temp = FIFOq_dequeue(created_queue);
    PCB_set_state(temp, ready);
    printf("Process enqueued: %s\n", PCB_to_string(temp));
    PRIORITYq_enqueue(ready_queue, temp);
  }

  // Determine what kind of interrupt happened
  switch(int_type) {
    case timer:
      printf("timer interrupt\n");
      return_status = timer_interrupt_handler();
      pcb_to_be_dispatched = round_robin();
      timer_count = 0; // reset the quantum timer
      break;
    case io_1_interrupt:
      printf("io 1 request\n");
      return_status = io_request_trap(io_1_interrupt);
      pcb_to_be_dispatched = round_robin();
      timer_count = 0; // reset the quantum timer
      break;
    case io_2_interrupt:
      printf("io 2 request\n");
      return_status = io_request_trap(io_2_interrupt);
      pcb_to_be_dispatched = round_robin();
      timer_count = 0; // reset the quantum timer
      break;
    case io_1_completion_interrupt:
      printf("io 1 completion\n");
      return_status = io_completion_interrupt_handler(io_1_completion_interrupt);
      pcb_to_be_dispatched = current_process;
      break;
    case io_2_completion_interrupt:
      printf("io 2 completion\n");
      return_status = io_completion_interrupt_handler(io_2_completion_interrupt);
      pcb_to_be_dispatched = current_process;
      break;
    case process_termination_interrupt:
      return_status = process_termination_trap_handler();
      pcb_to_be_dispatched = round_robin();
      timer_count = 0;
      break;
    case condition_wait_interrupt:
      return_status = condition_wait_handler();
      pcb_to_be_dispatched = round_robin();
      break;
    default: // this should never happen
      return_status = INVALID_INPUT;
      printf("Invalid interrupt type passed\n");
      break;
  }

  dispatcher(pcb_to_be_dispatched);
  return return_status;
}

// Checks if the current process should terminate
// Returns a 1 if it should
int check_terminate(void) { // simulates process throwing a terminate interrupt
  if (current_process != NULL) {
    if (pc_register == current_process->max_pc) { // if the current process is at it's max pc
      pc_register = 0;
      if (current_process->terminate > 0) { // processes with 0 never terminate
        current_process->term_count++; // increment the term count (how many times has it passed its max pc)
        if (current_process->term_count == current_process->terminate) { // if the term_count is equal to terminate, it is done
          return 1;
        }
      }
    }
    return 0;
  }
  printf("in process_termination_trap_handler: current_process was null\n");
  exit(NULL_POINTER);
  return NULL_POINTER;
}

int check_timer(void) {
  if (timer_count == (QUANTUM_DURATION - 1)) {
    return 1;
  }
  timer_count++;
  return 0;
}

int check_for_io_1_completion(void) {
  if (io_1_downcounter > 0) { // it's still downcounting
    io_1_downcounter--;
  } else if (io_1_downcounter == 0) { // io has completed
    io_1_downcounter = -1; // set to -1 so it doesn't keep downcounting and so it doesn't keep returning 1
    return 1;
  }
  return 0; // not done or not even running
}

int check_for_io_2_completion(void) {
  if (io_2_downcounter > 0) { // it's still downcounting
    io_2_downcounter--;
  } else if (io_2_downcounter == 0) { // io has completed
    io_2_downcounter = -1; // set to -1 so it doesn't keep downcounting and so it doesn't keep returning 1
    return 1;
  }
  return 0; // not done or not even running
}

int check_io_request(PCB_p the_pcb) {
  int i;
  if (pc_register > 0) { // 0 in an io array means no interrupt
    for (i = 0; i < 4; i++) {
      if (the_pcb->io_1_[i] == pc_register) {
        printf("Current PCB: pid#%lu, io 1\n", the_pcb->pid);
        return io_1_interrupt; // request for device number 1
      }
      if (the_pcb->io_2_[i] == pc_register) {
        printf("Current PCB: pid#%lu, io 2\n", the_pcb->pid);
        return io_2_interrupt; // request for device number 2
      }
    }
  }
  return 0;
}

int pseudo_isr(enum interrupt_type int_type) { //, unsigned long * cpu_pc_register) {
  // Pseudo push of PC to system stack
  sys_stack = pc_register;
  // printf("Setting sys_stack to what was in pc_register: %lu\n", sys_stack);

  if (current_process != NULL) {
    if (dispatch_counter == 4) {
      if (current_process->pid != idl->pid) {
        printf("\n\nCurrently Running PCB: %s\n", PCB_to_string(current_process));
      }
    }
    // Change state of current process to interrupted
    current_process->state = interrupted;

    // Save cpu state to current processes pcb
    // pc was pushed to sys_stack right before isr happened.
    current_process->pc = sys_stack;

    // Up-call to scheduler
    scheduler(int_type);

    // IRET, put sys_stack into pc_register
    pc_register = sys_stack;
    return NO_ERRORS;
  }
  printf("current_process was null\n"); //this should never happen
  return NULL_POINTER;
}

int get_producer_process_index(PCB_p the_pcb) {
  int i;
  for (i = 0; i < 5; i++) { // this is how we find out which variable to increment
    if (current_process->pid == producer[i]->pid) {
      return i;
    }
  }
  printf("Process not a producer or consumer. This should never happen\n"); // we should get a segfault from this error
  return -1;
}

int get_consumer_process_index(PCB_p the_pcb) {
  int i;
  for (i = 0; i < 5; i++) { // this is how we find out which variable to increment
    if (current_process->pid == consumer[i]->pid) {
      return i;
    }
  }
  printf("Process not a producer or consumer. This should never happen\n"); // we should get a segfault from this error
  return -1;
}

void cpu(void) {

  PCB_p temp_process;

  unsigned int * the_pointer; // used to increment the global variable
  int i, run_count;
  int random_pc_increment;
  int error_check;
  int rand_num_of_processes;
  int process_index;

  int descheduled_flag;

  int is_timer_interrupt;
  int is_io_request_interrupt;
  int is_io1_completion_interrupt;
  int is_io2_completion_interrupt;
  int is_terminate_state;
  pc_register = 0;

  // Final Project Create processes section
  //*************************************************************************
	create_producer_consumer_pairs();
  create_mutual_resource_pairs();
  create_compute_intensive_processes(25);
  //*************************************************************************
  // End final project Create processes section

  // main cpu loop
  // for (run_count = 0; run_count < RUN_TIME; run_count++) {
  do {

    //reset the checking values
    error_check = 0;
    is_io1_completion_interrupt = 0;
    is_io2_completion_interrupt = 0;
    is_io_request_interrupt = 0;
    is_terminate_state = 0;
    descheduled_flag = 0;

    if (create_count < CREATE_ITERATIONS) { // Create IO processes
      rand_num_of_processes = true_random(5);
      printf("Creating %d processes\n", rand_num_of_processes);
      total_processes_created += rand_num_of_processes;
      for (i = 0; i < rand_num_of_processes; i++) {
        PCB_p temp = PCB_construct();
        PCB_init(temp);
        PCB_randomize_IO_arrays(temp);
        temp->max_pc = (rand() % 4000) + 2000;
        temp->terminate = (rand() % 5) + 1; // TODO: + 1 is there so they all terminate, currently I want all to terminate
        temp->term_count = 0;
        PCB_set_pid(temp, pid_counter);
        pid_counter++;
        FIFOq_enqueue(created_queue, temp);
        printf("Process PID: %lu io arrays:\n", temp->pid);
        int arrcntr;
        for (arrcntr = 0; arrcntr < 4; arrcntr++) {
          printf("io1[%d]: %d, io2[%d]: %d\n", arrcntr, temp->io_1_[arrcntr], arrcntr, temp->io_2_[arrcntr]);
        }
      }
      create_count++;
    } // End create IO processes




    // Simulate running of current process (Execute instruction)
    pc_register++;

    //TODO: refactoring of process pseudo program

    if (current_process->role == PRODUCER) {
      // printf("Producer is running\n");
      process_index = get_producer_process_index(current_process);
      switch (pc_register) { // producer only instructions
        case FIRST_MUTEX_LOCK_INSTRUCTION:
          Mutex_lock(mutex[process_index], current_process);
          break;
        case FIRST_MUTEX_UNLOCK_INSTRUCTION:
          Mutex_unlock(mutex[process_index], ready_queue);
          break;
        case SECOND_MUTEX_LOCK_INSTRUCTION:
          Mutex_lock(mutex[process_index], current_process);
          break;
        case SECOND_MUTEX_UNLOCK_INSTRUCTION:
          Mutex_unlock(mutex[process_index], ready_queue);
          break;
        case WAIT_COND_INSTRUCTION:
          if (increment_avail[process_index] == 0) {
            cond_wait(num_not_incremented[process_index], mutex[process_index], current_process, ready_queue);
            //deschedule here
            descheduled_flag = 1;
          }
          break;
        case INCREMENT_INSTRUCTION:
          the_pointer = current_process->global_variable;
          (*the_pointer)++;
          break;
        case COND_SIGNAL_INSTRUCTION:
          printf("PID %lu sent signal on condition %p\n", current_process->pid, &increment_avail[process_index]);
          temp_process = cond_signal(num_not_printed[process_index]);
          if (temp_process != NULL) {
            PRIORITYq_enqueue(ready_queue, temp_process);
          }
          break;
        case CHANGE_INCREMENT_AVAIL_INSTRUCTION:
          increment_avail[process_index]--;
          break;
        default:
          break;
      }
    }

    if (current_process->role == CONSUMER) {
      // printf("Consumer is running\n");
      process_index = get_consumer_process_index(current_process);
      switch (pc_register) { // consumer only instructions
        case FIRST_MUTEX_LOCK_INSTRUCTION:
          Mutex_lock(mutex[process_index], current_process);
          break;
        case FIRST_MUTEX_UNLOCK_INSTRUCTION:
          Mutex_unlock(mutex[process_index], ready_queue);
          break;
        case SECOND_MUTEX_LOCK_INSTRUCTION:
          Mutex_lock(mutex[process_index], current_process);
          break;
        case SECOND_MUTEX_UNLOCK_INSTRUCTION:
          Mutex_unlock(mutex[process_index], ready_queue);
          break;
        case WAIT_COND_INSTRUCTION:
          if (increment_avail[process_index] == 1) {
            cond_wait(num_not_printed[process_index], mutex[process_index], current_process, ready_queue);
            descheduled_flag = 1;
          }
          break;
        case PRINT_INSTRUCTION:
          the_pointer = current_process->global_variable;
          printf("PID: %lu, sequence: %d\n", current_process->pid, (*the_pointer));
          break;
        case COND_SIGNAL_INSTRUCTION:
          printf("PID %lu sent signal on condition %p\n", current_process->pid, &increment_avail[process_index]);
          temp_process = cond_signal(num_not_incremented[process_index]);
          if (temp_process != NULL) {
            PRIORITYq_enqueue(ready_queue, temp_process);
          }
          break;
        case CHANGE_INCREMENT_AVAIL_INSTRUCTION:
          increment_avail[process_index]++;
          break;
        default:
          break;
      }
    }

    if (current_process->role == MUTUAL_RESOURCE_USER) {
      if (current_process->pid == mutual_resource_1[0]->pid) {
        switch (pc_register) {
          case LOCK_R1_INSTRUCTION:
            Mutex_lock(r1_mutual_resource_mutex[0], current_process);
            break;
          case LOCK_R2_INSTRUCTION:
            Mutex_lock(r2_mutual_resource_mutex[0], current_process);
            break;
          case PRINT_TO_TRACE_INSTRUCTION:
            printf("Both resources are used\n");
            break;
          case UNLOCK_R2_INSTRUCTION:
            Mutex_unlock(r2_mutual_resource_mutex[0], ready_queue);
            break;
          case UNLOCK_R1_INSTRUCTION:
            Mutex_unlock(r1_mutual_resource_mutex[0], ready_queue);
            break;
          default:
            break;
        }
      }
    }

    if (current_process->role == MUTUAL_RESOURCE_USER) {
      if (current_process->pid == mutual_resource_2[0]->pid) {
        switch (pc_register) {
          case LOCK_R1_INSTRUCTION:
            Mutex_lock(r1_mutual_resource_mutex[0], current_process);
            break;
          case LOCK_R2_INSTRUCTION:
            Mutex_lock(r2_mutual_resource_mutex[0], current_process);
            break;
          case PRINT_TO_TRACE_INSTRUCTION2:
            printf("Both resources are used\n");
            break;
          case UNLOCK_R2_INSTRUCTION:
            // Mutex_unlock(r2_mutual_resource_mutex[0], ready_queue);
            break;
          case UNLOCK_R1_INSTRUCTION:
            // Mutex_unlock(r1_mutual_resource_mutex[0], ready_queue);
            break;
          default:
            break;
        }
      }
    }


    // if (current_process->role == PRODUCER || current_process->role == CONSUMER) { // all the producer instructions
    //   process_index = get_producer_process_index(current_process);
    //   switch (pc_register) { // common instructions
    //     case FIRST_MUTEX_LOCK_INSTRUCTION:
    //       Mutex_lock(mutex[process_index], current_process);
    //       break;
    //     case FIRST_MUTEX_UNLOCK_INSTRUCTION:
    //       Mutex_unlock(mutex[process_index], ready_queue);
    //       break;
    //     case SECOND_MUTEX_LOCK_INSTRUCTION:
    //       Mutex_lock(mutex[process_index], current_process);
    //       break;
    //     case SECOND_MUTEX_UNLOCK_INSTRUCTION:
    //       Mutex_unlock(mutex[process_index], ready_queue);
    //       break;
    //     default:
    //       break;
    //   }
    //
    //   if (current_process->role == PRODUCER) {
    //     switch (pc_register) { // producer only instructions
    //       case WAIT_COND_INSTRUCTION:
    //         if (increment_avail[process_index] == 0) {
    //           cond_wait(num_not_incremented[process_index], mutex[process_index], ready_queue);
    //         }
    //         break;
    //       case INCREMENT_INSTRUCTION:
    //         the_pointer = current_process->global_variable;
    //         (*the_pointer)++;
    //         break;
    //       case COND_SIGNAL_INSTRUCTION:
    //         printf("PID %lu sent signal on condition %p\n", current_process->pid, &increment_avail[process_index]);
    //         cond_signal(num_not_printed[process_index]);
    //         break;
    //       case CHANGE_INCREMENT_AVAIL_INSTRUCTION:
    //         increment_avail[process_index]--;
    //         break;
    //       default:
    //         break;
    //     }
    //   } else { // it's a consumer
    //     switch (pc_register) { // consumer only instructions
    //       case WAIT_COND_INSTRUCTION:
    //         if (increment_avail[process_index] == 1) {
    //           cond_wait(num_not_incremented[process_index], mutex[process_index], ready_queue);
    //         }
    //         break;
    //       case PRINT_INSTRUCTION:
    //         the_pointer = current_process->global_variable;
    //         printf("PID: %lu, sequence: %d\n", current_process->pid, (*the_pointer));
    //         break;
    //       case COND_SIGNAL_INSTRUCTION:
    //         printf("PID %lu sent signal on condition %p\n", current_process->pid, &increment_avail[process_index]);
    //         cond_signal(num_not_printed[process_index]);
    //         break;
    //       case CHANGE_INCREMENT_AVAIL_INSTRUCTION:
    //         increment_avail[process_index]++;
    //         break;
    //       default:
    //         break;
    //     }
    //   }

    // }

    if (run_count % 1000 == 0) {
      deadlock_check();
    }

    if (descheduled_flag) { // condition wait was called and current process was descheduled
      error_check = pseudo_isr(condition_wait_interrupt);
    } else {
      // Check if process should be terminated
      // We don't technically need this, terminate_check has logic to stop from terminating idl
      // this just seems faster
      if (current_process->pid != idl->pid) { // idl process does not terminate
        is_terminate_state = check_terminate();
      }

      // Check for timer interrupt
      is_timer_interrupt = check_timer();

      // Check for I/O completions
      is_io1_completion_interrupt = check_for_io_1_completion();
      is_io2_completion_interrupt = check_for_io_2_completion();

      // Check for io requests
      is_io_request_interrupt = check_io_request(current_process);

      if (is_io1_completion_interrupt) {
        error_check = pseudo_isr(io_1_completion_interrupt);
      }
      if (is_io2_completion_interrupt) {
        error_check = pseudo_isr(io_2_completion_interrupt);
      }

      // Call pseudo_isr for interrupts, with priority: 1=terminate, 2=timer, 3=io_request
      // If the process shouldn't terminate, it can be requesting io (process that io request)
      // if it's not requesting io, the timer could be up (process timer interrupt)
      if (is_terminate_state) {
        error_check = pseudo_isr(process_termination_interrupt);
      } else if (is_io_request_interrupt) {
        error_check = pseudo_isr(is_io_request_interrupt);
      } else if (is_timer_interrupt) {
        error_check = pseudo_isr(timer);
      }
    }


    // call pseudo_isr with timer interrupt type
    switch(error_check) { // handle any errors that happened in pseudo_isr
      case NO_ERRORS: //working good, just break and continue;
        break;
      case NULL_POINTER:
        print_error(NULL_POINTER);
        break;
      case INVALID_INPUT:
        print_error(INVALID_INPUT);
        break;
      default:
        printf("Unknown error\n");
        break;
    }

	  run_count++;
  } while ((ready_queue->size > 0 || io_1_waiting_queue-> size > 0 ||
          io_2_waiting_queue->size > 0 || created_queue->size > 0 ||
          current_process->pid != idl->pid) && MAX_LOOPS != run_count );// End main cpu loop
}

int main(void) {

  // Seed rand
  srand(time(NULL));

  // initialize variables
  pid_counter = 0;
  sys_stack = 0;
  create_count = 0;
  dispatch_counter = 0;
  io_1_downcounter = -1;
  io_2_downcounter = -1;
  timer_count = 0;
  total_processes_created = 0;

  // Create and initialize idle pcb
  idl = PCB_construct();
  PCB_init(idl);
  PCB_set_pid(idl, 0xFFFFFFFF);
  current_process = idl;

  // Create and initialize queues
  ready_queue = PRIORITYq_construct();
  //available_index_queue = NUMBERq_construct();
  //NUMBERq_init(available_index_queue, MAX_MUTEX_SIZE);
  // PRIORITYq_init(ready_queue); // priority queue does not need to init
  created_queue = FIFOq_construct();
  FIFOq_init(created_queue);
  io_1_waiting_queue = FIFOq_construct();
  FIFOq_init(io_1_waiting_queue);
  io_2_waiting_queue = FIFOq_construct();
  FIFOq_init(io_2_waiting_queue);
  terminate_queue = FIFOq_construct();
  FIFOq_init(terminate_queue);

  // Run a cpu
  char * queue_string;
  cpu();


  printf("Ready Queue size: %d\n", ready_queue->size);
  printf("IO 1 Queue size: %d\n", io_1_waiting_queue->size);
  printf("IO 2 Queue size: %d\n", io_2_waiting_queue->size);
  printf("Terminated Queue size: %d\n", terminate_queue->size);
  queue_string = FIFOq_to_string(terminate_queue);
  printf("%s\n", queue_string);
  // free(queue_string);
  printf("Total processes created: %d\n", total_processes_created);

  // Cleanup / free stuff to not have memory leaks
  // PRIORITYq_destruct(ready_queue);
  // FIFOq_destruct(created_queue);
  // FIFOq_destruct(io_1_waiting_queue);
  // FIFOq_destruct(io_2_waiting_queue);
  // FIFOq_destruct(terminate_queue);

  // if (current_process != NULL) {
  //   if (current_process->pid != idl->pid) {
  //     printf("should never happen if all processes are set to terminate,\n idle process should always be the last thing running********\n");
  //     PCB_destruct(current_process);
  //   }
  // }
  // PCB_destruct(idl);

  return 0;

}
