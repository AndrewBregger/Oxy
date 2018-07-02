#include "queue.h"

QueueNode* new_node(void* data, QueueNode* next) {
	QueueNode* node = (QueueNode*) malloc(sizeof(QueueNode));
	node->data = data;
	node->next = next;
	return node;
}

Queue new_queue() {
	return (Queue) {0};
}

void push_queue(Queue* queue, void* data) {
	if(!queue->head) {
		queue->head = new_node(data, NULL);
		queue->tail = queue->head;
	}
	else {
		queue->tail->next = new_node(data, NULL);
		queue->tail = queue->tail->next;
	}

}
void* top_queue(Queue* queue) {
	if(queue->head)
		return queue->head->data;
	else
		return NULL;
}
void pop_queue(Queue* queue) {
	QueueNode* node = top_queue(queue);
	queue->head = node->next;
	free(node);
}