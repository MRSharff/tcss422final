#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "PCB.h"

// Function Prototypes
char* getStateName(State state);

PCB_p PCB_construct (void) {
    return malloc(sizeof(PCB));
}

void PCB_destruct(PCB_p pcb) {
    if (pcb != NULL) free(pcb);
}

int PCB_init(PCB_p pcb) {
    if (pcb == NULL) {
        return NULL_OBJECT;
    }
    pcb->pid = DEFAULT_PID;
    pcb->state = (State) DEFAULT_STATE;
    pcb->priority = DEFAULT_PRIORITY;
    pcb->pc = DEFAULT_PC;
    pcb->term_count = 0;
	pcb->boost = 0;
    return SUCCESS;
}

char* getStateName(State state) {
    switch(state) {
        case new:
            return "new";
        case ready:
            return "ready";
        case running:
            return "running";
        case interrupted:
            return "interrupted";
        case waiting:
            return "waiting";
        case halted:
            return "halted";
        case terminated:
            return "terminated";
    }
}

char * PCB_toString(PCB_p pcb) {
    char* string = malloc(100);
    if(pcb == NULL) {
        sprintf(string, "idle process");
    } else {
        sprintf(string, "PID: 0x%0lX, State: %s, Priority: 0x%0X, PC: 0x%04lX",
                pcb->pid, getStateName(pcb->state), pcb->priority, pcb->pc);
    }
    return string;
}
