#include "fifo_queue.h"
#include "PCB.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mythread.h"

#define SIMULATION_TICKS 100000   // The length of the test in CPU cycles.

#define QUANTUM 50
#define PCB_INIT_CNT 20
#define TIMER 1
#define TERMINATE 2
#define IO1 3
#define IO2 4

FIFOq_p readyPCBs;
FIFOq_p waitIO1;
FIFOq_p waitIO2;
FIFOq_p terminatedPCBs;

PCB_p currentPCB;

// Timers for each device.
// When the timer is at 1, the device throws an interrupt.
// When the timer is at 0, the device's timer is to its respective quantum.
int ticksRemaining_Timer;   // CPU ticks before next timer interrupt.
int ticksRemaining_IO1;   // CPU ticks before next I/O 1 interrupt.
int ticksRemaining_IO2;   // CPU ticks before next I/O 2 interrupt.
unsigned int paircount = 0;
FILE *fp;

// Function prototypes
void CPU_cycle();
void executeCurrentProcess();
void checkForInterrupt();
void TSR(int);
void scheduler();
void ISR(int);
void timerTick(int);
int ioRequested(unsigned long* traps, unsigned long PC);
void init();
void populateWithRandomPCBs(FIFOq_p queue, int amount);
void populateIOTrapArrays(PCB_p, int);


int main(void) {
	//write to file
	
	fp=fopen("c:\\Users\\sonvu\\Desktop\\test.txt", "wb");

    init();

    char* string = FIFOq_toString(readyPCBs);
    fprintf(fp,"%s%s", string,"\n");
    free(string);

    // Run simulation until all processes have been terminated.
    for(int i = 0; i < SIMULATION_TICKS; i++) {
        CPU_cycle();
    }

    fprintf(fp,"\nSimulation of %d processes complete.\n", PCB_INIT_CNT);
	fclose(fp);
    return 0;
}


void CPU_cycle() {
    if(currentPCB != NULL) {
        executeCurrentProcess();
    } else {
        // Check if a process has become ready.
        scheduler();
    }
    checkForInterrupt();
}

// Performs the following tasks relating to the execution of a process that occur during a single CPU cycle.
void executeCurrentProcess() {
    currentPCB->pc++;

    // If current PC exceeds max PC of the process, reset it, and check if process is to be terminated.
    if (currentPCB->pc > currentPCB->max_pc) {
        currentPCB->pc = 0;
        currentPCB->term_count++;

        // Check if the process is to be terminated.
        if (currentPCB->term_count == currentPCB->terminate) {
            TSR(TERMINATE);
            return; // No more execution needed.
        }
    }

    // Check if the process is to execute an I/O trap during this cycle.
    if(ioRequested(currentPCB->io_1_traps, currentPCB->pc)) {
        TSR(IO1);
    } else if(ioRequested(currentPCB->io_2_traps, currentPCB->pc)) {
        TSR(IO2);
    }
}

int ioRequested(unsigned long* traps, unsigned long PC) {
    for (int i = 0; i < IO_TRAPS; i++) {
        if(traps[i] == PC) {
            return 1;
        }
    }
    return 0;
}

void checkForInterrupt() {
    // Check for timer interrupt, call timer ISR if timer interrupt has occurred.
    if (ticksRemaining_Timer == 1) {
        ISR(TIMER);
    }
    timerTick(TIMER);

    // Check for I/O 1 interrupt, call I/O 1 ISR if I/O 1 interrupt has occurred.
    if (ticksRemaining_IO1 == 1) {
        ISR(IO1);
    }
    timerTick(IO1);

    // Check for I/O 2 interrupt, call I/O 2 ISR if I/O 2 interrupt has occurred.
    if (ticksRemaining_IO2 == 1) {
        ISR(IO2);
    }
    timerTick(IO2);
}

void TSR(int trap) {
    char* PCB_string;
    switch (trap) {
        case TIMER:
            currentPCB->state = ready;
            FIFOq_enqueue(readyPCBs, currentPCB);

            PCB_string = PCB_toString(currentPCB);
            fprintf(fp,"Timer interrupt during %s\n", PCB_string);
            free(PCB_string);
            break;
        case TERMINATE:
            currentPCB->state = terminated;
            currentPCB->termination = time(NULL);
            FIFOq_enqueue(terminatedPCBs, currentPCB);

            PCB_string = PCB_toString(currentPCB);
            fprintf(fp,"Terminating %s\n", PCB_string);
            free(PCB_string);

            currentPCB = NULL;
            break;
        case IO1:
            currentPCB->state = waiting;
            FIFOq_enqueue(waitIO1, currentPCB);

            PCB_string = PCB_toString(currentPCB);
            fprintf(fp,"I/O 1 Trap requested by %s\n", PCB_string);
            free(PCB_string);
            break;
        case IO2:
            currentPCB->state = waiting;
            FIFOq_enqueue(waitIO2, currentPCB);

            PCB_string = PCB_toString(currentPCB);
            fprintf(fp,"I/O 2 Trap requested by %s\n", PCB_string);
            free(PCB_string);
            break;
        default:
            return; // If an unknown trap requested, don't do anything.
    }

    scheduler();
}

void scheduler() {
    if(!FIFOq_is_empty(readyPCBs)) {
        currentPCB = FIFOq_dequeue(readyPCBs);
        currentPCB->state = running;
    } else {
        currentPCB = NULL;
    }

    char* PCB_string = PCB_toString(currentPCB);
    fprintf(fp,"Switching to %s\n", PCB_string);
    free(PCB_string);

    int running = FIFOq_size(readyPCBs);
    if(currentPCB != NULL) {
        running++;
    }
    fprintf(fp,"%d processes running\n", running);
    fprintf(fp,"%d processes awaiting completion of I/O 1\n", FIFOq_size(waitIO1));
    fprintf(fp,"%d processes awaiting completion of I/O 2\n", FIFOq_size(waitIO2));


    // Remove all terminated processes.
    while(!FIFOq_is_empty(terminatedPCBs)) {
        PCB_p terminatedPCB = FIFOq_dequeue(terminatedPCBs);

        PCB_string = PCB_toString(terminatedPCB);
        fprintf(fp,"Resources freed from %s\n", PCB_string);
        free(PCB_string);

        PCB_destruct(terminatedPCB);
    }
}

void ISR(int interrupt) {
    PCB_p completedIOPCB;
    char* PCB_string;

    switch (interrupt) {
        case TIMER:
            TSR(TIMER);
            break;
        case IO1:
            completedIOPCB = FIFOq_dequeue(waitIO1);
            completedIOPCB->state = ready;
            FIFOq_enqueue(readyPCBs, completedIOPCB);

            PCB_string = PCB_toString(completedIOPCB);
            fprintf(fp,"I/O 1 Trap request completed for %s\n", PCB_string);
            free(PCB_string);
            break;
        case IO2:
            completedIOPCB = FIFOq_dequeue(waitIO2);
            completedIOPCB->state = ready;
            FIFOq_enqueue(readyPCBs, completedIOPCB);

            PCB_string = PCB_toString(completedIOPCB);
            fprintf(fp,"I/O 2 Trap request completed for %s\n", PCB_string);
            free(PCB_string);
            break;
        default:
            return; // If an unknown device threw an interrupt, don't do anything.
    }

}


void timerTick(int device) {
    switch (device) {
        case TIMER:
            if (FIFOq_is_empty(readyPCBs)) {
                ticksRemaining_Timer = 0;
            } else if (ticksRemaining_Timer == 0) {
                ticksRemaining_Timer = QUANTUM;
            } else {
                ticksRemaining_Timer--;
            }
            break;
        case IO1:
            if (FIFOq_is_empty(waitIO1)) {
                ticksRemaining_IO1 = 0;
            } else if(ticksRemaining_IO1 <= 0) {
                ticksRemaining_IO1 = QUANTUM * ((3 % rand()) + 2);
            } else{
                ticksRemaining_IO1--;
            }
            break;
        case IO2:
            if (FIFOq_is_empty(waitIO2)) {
                ticksRemaining_IO2 = 0;
            } else if(ticksRemaining_IO2 <= 0) {
                ticksRemaining_IO2 = QUANTUM * ((3 % rand()) + 2);
            } else {
                ticksRemaining_IO2--;
            }
            break;
        default:
            return; // If an unknown device is to be ticked down, don't do anything.
    }
}

void init() {
    // Setup system functions.
    srand((unsigned int) time(NULL));

    // Setup timers and counters.
    ticksRemaining_Timer = QUANTUM;
    ticksRemaining_IO1 = 0;   // I/O 1 wait queue initially empty.
    ticksRemaining_IO2 = 0;   // I/O 2 wait queue initially empty.

    // Create and populate queue of ready PCBs.
    readyPCBs = FIFOq_construct();
    FIFOq_init(readyPCBs);
    populateWithRandomPCBs(readyPCBs, PCB_INIT_CNT);

    // Create queues for both the I/O devices and the termination queue.
    waitIO1 = FIFOq_construct();
    FIFOq_init(waitIO1);
    waitIO2 = FIFOq_construct();
    FIFOq_init(waitIO2);
    terminatedPCBs = FIFOq_construct();
    FIFOq_init(terminatedPCBs);

    // The first PCB to be run is the first ready PCB.
    currentPCB = FIFOq_dequeue(readyPCBs);
}

// Populates the passed queue with the passed amount of randomly generated PCBs.
void populateWithRandomPCBs(FIFOq_p queue, int amount) {
    // Note that PIDCount is static; it will be initialized to 0 only the first time that this function is called.
    unsigned static long PIDCount = 0; // The PID of the next PCB that will be created.

    for (int i = 0; i < amount; i++) {
        PCB_p newPCB = PCB_construct();
        PCB_init(newPCB);   // TODO: Consider removal.
        newPCB->pid = ++PIDCount;
        newPCB->priority = (unsigned short) (rand() % 5);
        newPCB->state = ready;
        newPCB->pc = 0;
        newPCB->sw = 0;
        newPCB->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        newPCB->creation = time(NULL);
        newPCB->termination = -1;   // Not terminated yet.
        newPCB->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        newPCB->term_count = 0;
		newPCB->role = 0;
		newPCB->boost = 0;
        populateIOTrapArrays(newPCB, 1);
        populateIOTrapArrays(newPCB, 2);
        FIFOq_enqueue(queue, newPCB);
    }
	
	//create pairs
        PCB_p pro1 = PCB_construct();
        PCB_init(pro1);   // TODO: Consider removal.
        pro1->pid = ++PIDCount;
        pro1->priority = 0;
        pro1->state = ready;
        pro1->pc = 0;
        pro1->sw = 0;
        pro1->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        pro1->creation = time(NULL);
        pro1->termination = -1;   // Not terminated yet.
        pro1->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        pro1->term_count = 0;
		pro1->role = 2;
		pro1->boost = 0;
        FIFOq_enqueue(queue, pro1);
		
		PCB_p con1 = PCB_construct();
        PCB_init(con1);   // TODO: Consider removal.
        con1->pid = ++PIDCount;
        con1->priority = 0;
        con1->state = ready;
        con1->pc = 0;
        con1->sw = 0;
        con1->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        con1->creation = time(NULL);
        con1->termination = -1;   // Not terminated yet.
        con1->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        con1->term_count = 0;
		con1->role = 1;
		con1->boost = 0;
        FIFOq_enqueue(queue, con1);
		
		        PCB_p pro2 = PCB_construct();
        PCB_init(pro2);   // TODO: Consider removal.
        pro2->pid = ++PIDCount;
        pro2->priority = 0;
        pro2->state = ready;
        pro2->pc = 0;
        pro2->sw = 0;
        pro2->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        pro2->creation = time(NULL);
        pro2->termination = -1;   // Not terminated yet.
        pro2->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        pro2->term_count = 0;
		pro2->role = 2;
		pro2->boost = 0;
        FIFOq_enqueue(queue, pro2);
		
		PCB_p con2 = PCB_construct();
        PCB_init(con2);   // TODO: Consider removal.
        con2->pid = ++PIDCount;
        con2->priority = 0;
        con2->state = ready;
        con2->pc = 0;
        con2->sw = 0;
        con2->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        con2->creation = time(NULL);
        con2->termination = -1;   // Not terminated yet.
        con2->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        con2->term_count = 0;
		con2->role = 1;
		con2->boost = 0;
        FIFOq_enqueue(queue, con2);
		
		        PCB_p pro3 = PCB_construct();
        PCB_init(pro3);   // TODO: Consider removal.
        pro3->pid = ++PIDCount;
        pro3->priority = 0;
        pro3->state = ready;
        pro3->pc = 0;
        pro3->sw = 0;
        pro3->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        pro3->creation = time(NULL);
        pro3->termination = -1;   // Not terminated yet.
        pro3->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        pro3->term_count = 0;
		pro3->role = 2;
		pro3->boost = 0;
        FIFOq_enqueue(queue, pro3);
		
		PCB_p con3 = PCB_construct();
        PCB_init(con3);   // TODO: Consider removal.
        con3->pid = ++PIDCount;
        con3->priority = 0;
        con3->state = ready;
        con3->pc = 0;
        con3->sw = 0;
        con3->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        con3->creation = time(NULL);
        con3->termination = -1;   // Not terminated yet.
        con3->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        con3->term_count = 0;
		con3->role = 1;
		con3->boost = 0;
        FIFOq_enqueue(queue, con3);
		
		PCB_p pro4 = PCB_construct();
        PCB_init(pro4);   // TODO: Consider removal.
        pro4->pid = ++PIDCount;
        pro4->priority = 0;
        pro4->state = ready;
        pro4->pc = 0;
        pro4->sw = 0;
        pro4->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        pro4->creation = time(NULL);
        pro4->termination = -1;   // Not terminated yet.
        pro4->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        pro4->term_count = 0;
		pro4->role = 2;
		pro4->boost = 0;
        FIFOq_enqueue(queue, pro4);
		
		PCB_p con4 = PCB_construct();
        PCB_init(con4);   // TODO: Consider removal.
        con4->pid = ++PIDCount;
        con4->priority = 0;
        con4->state = ready;
        con4->pc = 0;
        con4->sw = 0;
        con4->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        con4->creation = time(NULL);
        con4->termination = -1;   // Not terminated yet.
        con4->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        con4->term_count = 0;
		con4->role = 1;
		con4->boost = 0;
        FIFOq_enqueue(queue, con4);
		
		PCB_p pro5 = PCB_construct();
        PCB_init(pro5);   // TODO: Consider removal.
        pro5->pid = ++PIDCount;
        pro5->priority = 0;
        pro5->state = ready;
        pro5->pc = 0;
        pro5->sw = 0;
        pro5->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        pro5->creation = time(NULL);
        pro5->termination = -1;   // Not terminated yet.
        pro5->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        pro5->term_count = 0;
		pro5->role = 2;
		pro5->boost = 0;
        FIFOq_enqueue(queue, pro5);
		
		PCB_p con5 = PCB_construct();
        PCB_init(con5);   // TODO: Consider removal.
        con5->pid = ++PIDCount;
        con5->priority = 0;
        con5->state = ready;
        con5->pc = 0;
        con5->sw = 0;
        con5->max_pc = (unsigned long) (rand() % MAX_PC_VAL); // Set a randomly assigned max PC.
        con5->creation = time(NULL);
        con5->termination = -1;   // Not terminated yet.
        con5->terminate = (unsigned int) (rand() % MAX_TERMINATION_COUNT); // Set a randomly assigned termination count.
        con5->term_count = 0;
		con5->role = 1;
		con5->boost = 0;
        FIFOq_enqueue(queue, con5);
}

// Populates the passed I/O device's array of the passed PCB with random PC values.
// Ensures that PC values are unique.
void populateIOTrapArrays(PCB_p pcb, int ioDevice) {
    unsigned long max = pcb->max_pc;

    unsigned long num0 = rand() % max;
    unsigned long num1 = rand() % max;
    unsigned long num2 = rand() % max;
    unsigned long num3 = rand() % max;

    // Ensure num1 is unique.
    while (num1 == num0) {
        num1 = rand() % max;
    }

    // Ensure num2 is unique.
    while (num2 == num0 || num2 == num1) {
        num2 = rand() % max;
    }

    // Ensure num3 is unique.
    while (num3 == num0 || num3 == num1 || num3 == num2) {
        num3 = rand() % max;
    }

    // Set it to appropriate array in PCB.
    // This is very poor practice, but it avoids memory leaks.
    // If time permits we can convert the arrays to be instantiated here, and destroyed in PCB_destruct().
    if (ioDevice == 1) {
        pcb->io_1_traps[0] = num0;
        pcb->io_1_traps[1] = num1;
        pcb->io_1_traps[2] = num2;
        pcb->io_1_traps[3] = num3;
    }
    else if (ioDevice == 2) {
        pcb->io_2_traps[0] = num0;
        pcb->io_2_traps[1] = num1;
        pcb->io_2_traps[2] = num2;
        pcb->io_2_traps[3] = num3;
    }

}