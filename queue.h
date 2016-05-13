#ifndef QUEUE_H_
#define QUEUE_H_
#include "PCB.h"

#define DEFAULT_SIZE 0
#define DEFAULT_COUNTER 0



typedef struct node {
    PCB *pcb;
    struct node *next;
}Node;

typedef struct queue {
    Node *head, *rear;
    int size, counter;
}Queue;

typedef Queue * Queue_q;
Queue_q queue_construct (void);
int queue_init (Queue_q);
void queue_destruct (Queue_q);
Queue * enqueue (Queue *head, PCB *new);
PCB * dequeue(Queue *head);
PCB * peek(Queue *head);
int isEmpty(Queue *head);
void toString(Queue *head);
#endif
