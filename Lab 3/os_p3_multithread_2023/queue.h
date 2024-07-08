#ifndef MY_QUEUE
#define MY_QUEUE

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef struct __elem {
	void * value_ptr;
	struct __elem * next;
	struct __elem * prev;
} q_elem;

typedef struct {
	// Define the struct yourself
	int max_length;
	int current_length;
	q_elem * head;
	q_elem * tail;
} queue;

queue* queue_init (int length);
int queue_destroy (queue* q);
int queue_put (queue* q, void* elem);
void* queue_get(queue* q);
int queue_empty(queue* q);
int queue_full(queue* q);

#endif
