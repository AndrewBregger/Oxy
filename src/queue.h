#ifndef QUEUE_H_
#define QUEUE_H_

#include "common.h"

typedef struct QueueNode {
	void* data;
	struct QueueNode* next;
} QueueNode;

typedef struct Queue {
	QueueNode* head;
	QueueNode* tail;
	u32 size;
} Queue;

Queue new_queue();
void push_queue(Queue* queue, void* data);
void* top_queue(Queue* queue);
void pop_queue(Queue* queue);

#endif