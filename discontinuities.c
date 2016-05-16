//Justin Clark, Bincheng Li, Joshua Cho, Alexander Orozco

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "discontinuities.h"

static unsigned int SysStack = 0;
static int CallsToDispathcer = 0;


pthread_mutex_t timer1_mutex, timer2_mutex, io1_mutex, io2_mutex, trap1_mutex, trap2_mutex, mutex3_mutex, mutex4_mutex;

pthread_t pthr_timer, pthr_iotrap1, pthr_iotrap2;

int randomNumber (int min, int max) {
	int r;
	srand(time(NULL));
	r = (random() % max - min) + min;
	return r;
}

int main (void) {
	CPU cpu;
	cpu.mainLoop = mainloopFunction;
	cpu.mainLoop(&cpu);
	printf("\nProgram completed.\n\n(Note)Interrupts and matches can be found with ========================================= \nfollowing their announcements.\n");
	return 1;
}

void mainloopFunction(struct cpu *self) {
	unsigned long pid = 1;
	int IO_1TrapFound = 0;
	int IO_2TrapFound = 0;

	
	pthread_mutex_init(&timer1_mutex, NULL);
	pthread_mutex_init(&timer2_mutex, NULL);
	pthread_mutex_init(&io1_mutex, NULL);
	pthread_mutex_init(&io2_mutex, NULL);
	pthread_mutex_init(&trap1_mutex, NULL);
	pthread_mutex_init(&trap2_mutex, NULL);
	pthread_mutex_init(&mutex3_mutex, NULL);
	pthread_mutex_init(&mutex4_mutex, NULL);
	
	
	pthread_mutex_lock(&timer2_mutex);
	pthread_mutex_lock(&trap1_mutex);
	pthread_mutex_lock(&trap2_mutex);
	
	
	
	
	if (pthread_create(&pthr_timer, NULL, Timer, NULL)){
		fprintf(stderr, "Can't create timer thread\n");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_create(&pthr_iotrap1, NULL, &io_trap1, NULL)){
		fprintf(stderr, "Can't create IO trap1 thread\n");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_create(&pthr_iotrap2, NULL, &io_trap2, NULL)){
		fprintf(stderr, "Can't create IO trap2 thread\n");
		exit(EXIT_FAILURE);
	}
	
	
	Queue_q readyQueue = queue_construct();
	queue_init(readyQueue);
	
	Queue_q newProcessQueue = queue_construct();
	queue_init(newProcessQueue);
	
	Queue_q terminateQueue = queue_construct();
	queue_init(terminateQueue);
	
	Queue_q waitingQueueIO1 = queue_construct();
	queue_init(waitingQueueIO1);
	
	Queue_q waitingQueueIO2 = queue_construct();
	queue_init(waitingQueueIO2);
	
	PCB_p currProcess = PCB_construct();
	PCB_init(currProcess);
	PCB_set_pid(currProcess, pid);
	int i;
	for(i = 0; i < DEFAULT_NUM_PCBS; i++) {
			PCB_p p = PCB_construct();
			PCB_init(p);
			pid++;
			PCB_set_pid(p, pid);
			newProcessQueue = enqueue(newProcessQueue, p);
	}
	
		NewToReady(newProcessQueue, readyQueue);
		int executions = 0;
	while (executions < DEFAULT_NUM_EXECUTIONS) {
		executions++;
		
		printf("\nreadyQueue size = %i\nIO1queue size = %i\nIO2queue size = %i\nterminateQueue size = %i\n", readyQueue->size, waitingQueueIO1->size, waitingQueueIO2->size, terminateQueue->size);
		self->pcRegister = PCB_get_pc(currProcess);
		printf("\ncurrProcess pid = %lu pc = %lu\n", PCB_get_pid(currProcess), PCB_get_pc(currProcess));
		SysStack = self->pcRegister;
		
		if (PC_Increment(currProcess) == 1) { //If incrementing the PC causes termcount = terminate then puts PCB in terminate queue and dequeues new currProcess.
			PCB_set_state(currProcess, terminated);
			printf("\nProcess pid = %lu terminated. \n", PCB_get_pid(currProcess));
			currProcess->termination = time(NULL);
			terminateQueue = enqueue(terminateQueue, currProcess);
			
			if (readyQueue->size > 0) {
				currProcess = dequeue(readyQueue);
			} else currProcess = NULL;	
			self->pcRegister = PCB_get_pc(currProcess);
			SysStack = self->pcRegister;
		}
		printf("\ncurrProcess pid = %lu new pc = %lu\n", PCB_get_pid(currProcess), PCB_get_pc(currProcess));
		
		
//	The following if statements check for signals in all 3 threads while timer has priority. If timer mutex is unlocked goes to ISR, if IO unlocked goes to IOTrapHandler.
	
		if (pthread_mutex_trylock(&timer1_mutex) == 0)  { // If timer mutex is open
			printf("\ntimer interrupt=======================================================================================\n");
			currProcess = ISR(timer, currProcess, readyQueue);
			pthread_mutex_unlock(&timer1_mutex);
			pthread_mutex_unlock(&timer2_mutex);
		}
		else if(pthread_mutex_trylock(&io1_mutex) == 0) {
			if(waitingQueueIO1->size > 0) {
				printf("\nIO1 interrupt=======================================================================================\n");
				readyQueue = enqueue(readyQueue, dequeue(waitingQueueIO1));
				IO_1TrapFound = 0;
				pthread_mutex_lock(&trap1_mutex);
				pthread_mutex_unlock(&io1_mutex);
			}
		}
		
		else if (pthread_mutex_trylock(&io2_mutex) == 0) {
			if(waitingQueueIO2->size > 0) {
				printf("\nIO2 interrupt=======================================================================================\n");
				readyQueue = enqueue(readyQueue, dequeue(waitingQueueIO2)) ;
				IO_2TrapFound = 0;
				pthread_mutex_lock(&trap2_mutex);
				pthread_mutex_unlock(&io2_mutex);
			}
		}
		
		
		else if (IO_1TrapFound == 0) {
			if (CheckIOTrap1(currProcess) == 1) {
			IO_1TrapFound = 1;
			currProcess = IOTrapHandler(1, waitingQueueIO1, readyQueue, currProcess);
			pthread_mutex_lock(&mutex3_mutex);
			
			pthread_mutex_unlock(&trap1_mutex);
			}
		}
		else if (IO_2TrapFound == 0) {
			if (CheckIOTrap2(currProcess) == 1) {
			IO_2TrapFound = 1;
			currProcess = IOTrapHandler(2, waitingQueueIO2, readyQueue, currProcess);
			pthread_mutex_lock(&mutex4_mutex);
			
			pthread_mutex_unlock(&trap2_mutex);
			}
		}
				pthread_mutex_unlock(&mutex3_mutex);
				pthread_mutex_unlock(&mutex4_mutex);
	}
	
	while (readyQueue->size > 0) {
		PCB_destruct(dequeue(readyQueue));
	}
	
	queue_destruct(readyQueue);
	queue_destruct(newProcessQueue);
	queue_destruct(terminateQueue);
	
}

int CheckIOTrap1(PCB_p currProcess) { //goes through IO trap array in currProcess and checks if equal to PC
	int i = 0, trap;
	int pc = PCB_get_pc(currProcess);
	if (currProcess->IO_1Trap[i]  == pc) {
		return 1;
	}
	while (currProcess->IO_1Trap[i] != pc && i < 10) {
		trap = currProcess->IO_1Trap[i];
		i++;
		trap = currProcess->IO_1Trap[i];
		if (trap < 0) {
			return 0;
		}
		else if (currProcess->IO_1Trap[i]  == pc) {
			printf("\nMATCH pc = %lu IO_1Trap = %i=======================================================================================", currProcess->pc, currProcess->IO_1Trap[i]);
			return 1;
		} 
	}
	return 0;
}
int CheckIOTrap2(PCB_p currProcess) { //goes through IO trap array in currProcess and checks if equal to PC
	int i = 0, trap;
	int pc = PCB_get_pc(currProcess);
	if (currProcess->IO_2Trap[i]  == pc) {
		return 1;
	}
	while (currProcess->IO_2Trap[i] != pc && i < 10) {
		trap = currProcess->IO_2Trap[i];
		i++;
		trap = currProcess->IO_2Trap[i];
		if (trap < 0) {
			return 0;
		}
		else if (currProcess->IO_2Trap[i]  == pc) {
			printf("\nMATCH pc = %lu IO_2Trap = %i=======================================================================================", currProcess->pc, currProcess->IO_2Trap[i]);
			return 1;
		} 
	}
	return 0;
}


PCB_p IOTrapHandler(int IONumber, Queue_q waitingQ, Queue_q readyQueue, PCB_p currProcess) {
	switch(IONumber) {
		case 1:
		PCB_set_state(currProcess, halted);
		PCB_set_pc(currProcess, SysStack);
		waitingQ = enqueue(waitingQ, currProcess);
		//signal IO thread 1 to sleep
		return scheduler(IO, currProcess, readyQueue);
		
		case 2: 
		PCB_set_state(currProcess, halted);
		PCB_set_pc(currProcess, SysStack);
		enqueue(waitingQ, currProcess);
		//signal IO thread 2 to sleep
		return scheduler(IO, currProcess, readyQueue);
	}
	return NULL;
	
}

void NewToReady(Queue_q newQ, Queue_q readyQ) {
	
	while (newQ->size > 0) {
		
		PCB_p temp = dequeue(newQ);
		PCB_set_state(temp, ready);
		enqueue(readyQ, temp);
	}
}

PCB_p ISR(enum interrupt_type i, PCB_p currProcess, Queue_q readyQueue) {
	
	PCB_set_state(currProcess, interrupted);
	PCB_set_pc(currProcess, SysStack);
	
	return scheduler(i, currProcess, readyQueue);
}

PCB_p scheduler(enum interrupt_type i, PCB_p currProcess, Queue_q readyQueue) {
	
	switch (i) {
		case timer: 
			PCB_set_state(currProcess, ready);
			
			printf("Returned to Ready Queue: ");
			PCB_toString(currProcess);
			
			enqueue(readyQueue, currProcess);
			
			return dispatcher(currProcess, readyQueue);
		
		case IO:
			return dispatcher(currProcess, readyQueue);
			break;
			
		case interrupt:
			break;
	}
	return NULL;
}

PCB_p dispatcher(PCB_p currProcess, Queue_q readyQueue) {
	CallsToDispathcer++;
	
	if (CallsToDispathcer % 4 == 0) {
		return RoundRobinPrint(currProcess, readyQueue);
	}
	
	else {
		if (readyQueue->size < 1) {
			return NULL;
		}
		PCB_p newCurrentProcess = dequeue(readyQueue);
		
		PCB_set_state(newCurrentProcess, running);
		
		SysStack = PCB_get_pc(newCurrentProcess);
		
		return newCurrentProcess;
	}
}

PCB_p RoundRobinPrint(PCB_p currProcess, Queue_q readyQueue) {
		printf("\n\nDispathcher information:\n\nCurrent Progress: ");
		PCB_toString(currProcess);
		printf("Switching to: ");
		PCB_toString(peek(readyQueue));
		printf("\n");
		toString(readyQueue);
		PCB_p newCurrentProcess = dequeue(readyQueue);
		PCB_set_state(newCurrentProcess, running);
		SysStack = PCB_get_pc(newCurrentProcess);
		printf("\nNow Running: ");
		PCB_toString(newCurrentProcess);
		printf("\n\n");
		return newCurrentProcess;
}

void *Timer(void *args) {
	
	pthread_mutex_lock(&timer1_mutex);
	int Counter;
	struct timespec * time;

	time = (struct timespec*) calloc(1, sizeof(time));

	// if (time == NULL) {
	// 	// print_error(NULL_POINTER);
	// 	printf("Time was null");
	// }

	time->tv_nsec = 0.1;
	
	while(1) {
		Counter = QUANTUM;
		while (Counter > 0) {

			nanosleep(time, NULL);
			Counter--; 
		}
		pthread_mutex_unlock(&timer1_mutex);
		// Call ISR with timer interrupt
		while(pthread_mutex_trylock(&timer2_mutex) != 0) {
			
		}
		pthread_mutex_unlock(&timer2_mutex);
		pthread_mutex_lock(&timer1_mutex);
	}
	free(time);
	return NULL;
	
}
void *io_trap1(void *args) {
	
	pthread_mutex_lock(&io1_mutex);
	int ioCounter = (QUANTUM * 3 + QUANTUM*(rand()%2));
	while(1) {
		if(!pthread_mutex_trylock(&trap1_mutex)) {
			while (ioCounter > 0){
				//nanosleep((const struct timespec[]){{0, 500L}}, NULL);
				ioCounter--;
			}
			pthread_mutex_unlock(&io1_mutex);
			pthread_mutex_unlock(&trap1_mutex);
			
			ioCounter = (QUANTUM * 3 + QUANTUM*(rand()%2));
			
			
			while(pthread_mutex_trylock(&mutex3_mutex)) {
				
			}
			
			while(pthread_mutex_trylock(&io1_mutex)) {
				
			}
			
			pthread_mutex_unlock(&mutex3_mutex);
			
			
		}
			
		
	}
	return NULL;
		// call ISR with io interrupt
}

void *io_trap2(void *args) {
	
	pthread_mutex_lock(&io2_mutex);
	int ioCounter = (QUANTUM * 3 + QUANTUM*(rand()%2));
	while(1) {
		if(!pthread_mutex_trylock(&trap2_mutex)) {
			while (ioCounter > 0){
				//nanosleep((const struct timespec[]){{0, 500L}}, NULL);
				ioCounter--;
			}
			pthread_mutex_unlock(&io2_mutex);
			pthread_mutex_unlock(&trap2_mutex);
			ioCounter = (QUANTUM * 3 + QUANTUM*(rand()%2));
			
			while(pthread_mutex_trylock(&mutex4_mutex)) {
				
			}
			
			
			while(pthread_mutex_trylock(&io2_mutex)) {
				
			}
			
			pthread_mutex_unlock(&mutex4_mutex);
			
			
		}
			
		
	}
	return NULL;
		// call ISR with io interrupt
}

