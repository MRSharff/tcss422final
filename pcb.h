#ifndef PCB_H_
#define PCB_H_

#include "errors.h"

/*
	PCB helper class containing definitions, struct, and prototypes.
	
	@version 1
	4/8/16
	@author Joshua Cho
*/

#define DEFAULT_PID 0x1
#define DEFAULT_STATE created
#define DEFAULT_PRIORITY 15
#define DEFAULT_PC 0x0000
#define DEFAULT_MAXPC 2345
#define TERMINATE_COUNT 1

enum state_type {created, ready, running, interrupted, waiting, halted, terminated};	

typedef struct pcb {
    unsigned long pid;        // process ID #, a unique number
	enum state_type state;    // process state (running, waiting, etc.)
	unsigned short priority;  // priorities 0=highest, 15=lowest
	unsigned long pc;         // holds the current pc value when preempted
	int maxpc;					//Max value for PC
	int creation; 		//Time of creation for the PCB ***TIME will return a string not a number value in C
	int termination;	//Time the pcb is terminated 
	unsigned int terminate; //How many cycles until it terminates ie reached maxpc and resets x times
	unsigned int termCount; //How many times we've reached max PC
	int IO_1Trap[10];		//Trap handler calls
	int IO_2Trap[10];			//Trap handler calls
	
} PCB;

typedef PCB * PCB_p;

void IO_Trap_init(PCB_p); 

PCB_p PCB_construct (void); // returns a pcb pointer to heap allocation

void PCB_destruct (PCB_p);  // deallocates pcb from the heap

int PCB_init (PCB_p);       // sets default values for member data

int PCB_set_pid (PCB_p, unsigned long);	//sets pid of the pcb

unsigned long PCB_get_pid (PCB_p);  // returns pid of the process

int PCB_set_state (PCB_p, enum state_type);	//sets state of the pcb

unsigned short PCB_get_priority (PCB_p); 	//returns priority of the pcb

int PCB_set_priority (PCB_p, unsigned short);	//sets priority of the pcb

int PCB_set_pc (PCB_p, unsigned long);	//sets pc of the pcb

unsigned long PCB_get_pc (PCB_p); 	//returns pc of the pcb

const char* getStateName(PCB_p);	//returns char* pointing to string of state

void PCB_toString (PCB_p);
//void PCB_toString (PCB_p, char*);  // returns a string representing the contents of the pcb

// void print_error(int err);	//prints error information

int PC_Increment(PCB_p); //Function to add 1 to the PCB and handel the maxPC interaction returns 0 if not terminated, 1 if terminated

#endif