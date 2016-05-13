#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

Queue_q queue_construct (void) {// returns a queue pointer to heap allocation
	Queue_q q = (Queue_q) malloc (sizeof(Queue));
	return q;
}

void queue_destruct (Queue_q q) { 
	free(q);
}

int queue_init (Queue_q q) {      // sets default values for member data

		q->size = DEFAULT_SIZE;
		q->counter = DEFAULT_COUNTER;
		return 0;
	
}

Queue * enqueue (Queue *queue, PCB *pcb) {
    // Set up a new node to add to the list.
    Node *node = malloc(sizeof(Node));
    node->pcb = pcb;
    node->next = NULL;
    // Node is the front of the queue if it is empty, or it moves to the back.
    if (queue->size == 0) {
		queue->head = queue->rear = node;
	}
	else queue->rear->next = node;
    // An inserted node will be the new rear node.
    queue->rear = node;
    // Size of the queue and PID counter
    queue->size++;
    queue->counter++;
    return queue;
}

// Method for the dequeue
PCB * dequeue(Queue *queue) {
    if (queue->size == 0) {
        printf("Queue is empty\n");
        return NULL;
    } 
	else {
        queue->size--;
        PCB *returnPcb = queue->head->pcb;
        queue->head = queue->head->next;
		queue->counter--;
        return returnPcb;
    }
}

// Method to peek
PCB * peek(Queue *queue) {
	return queue->head->pcb;
}

// Method to check wheather or not a queue is empty (boolean)
int isEmpty(Queue *queue) {
    return queue->size == 0;
}

 // Method to print out the queue.
void toString(Queue *queue) {
    if (queue->size == 0) {
        printf("Queue is empty!\n");
    } else {
        Node *current = queue->head;
		printf("Queue Size = %i: \n", queue->size);
        while (current != NULL) {
            printf("PID: 0x%lu", current->pcb->pid);
            if (current->next != NULL) printf(" -> ");
            current = current->next;
            if(current == NULL) printf("-*\n");
        }
		//printf(" : contents: PID: 0x%lx, Priority: 0x%x, state: %i, PC: 0x%lx\n", 
		//		PCB_get_pid(queue->rear->pcb), PCB_get_priority(queue->rear->pcb), queue->rear->pcb->state, PCB_get_pc(queue->rear->pcb));
    }
}