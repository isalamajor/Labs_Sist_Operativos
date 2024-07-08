//OS-P3 2022-2023
#include "queue.h"
#include "parser.h"

//To create a queue
queue * queue_init(int size){
	queue * q = (queue *)malloc(sizeof(queue));
	q->max_length = size;
	q->head = NULL;
	q->tail = NULL;
	q->current_length = 0;
	return q;
}


// To Enqueue an element.
int queue_put(queue * q, void * value_ptr) {
	if (queue_full(q)) return -1;

	q_elem * elem = (q_elem *)malloc(sizeof(q_elem));
	elem->value_ptr = value_ptr;

	elem->next = NULL;
	elem->prev = NULL;

	if (queue_empty(q)) {
		q->head = elem;
		q->tail = elem;
	} else {
		q->tail->next = elem;
		elem->prev = q->tail;
		q->tail = elem;
	}
	q->current_length++;
	return 0;
}


// To Dequeue an element. It returns a void pointer that corresponds
// to the a pointer of the type of the element inserted in the queue
void * queue_get(queue * q) {
	if (queue_empty(q)) return NULL;

	q_elem * target = q->head;
	q->head = target->next;
	if (q->current_length == 1) {
		q->tail = NULL;
	} else {
		q->head->prev = NULL;
	}
	q->current_length--;

	op_t * elem = (op_t*) target->value_ptr;
	// We must free this struct, because it was only a wrapper for the actual value
	free(target);
	return elem;
}


//To check queue state
int queue_empty(queue * q){
	return (q->current_length == 0);
}

int queue_full(queue * q){
	return (q->max_length == q->current_length);
}

//To destroy the queue and free the resources
int queue_destroy(queue * q){
	if (q->head == NULL) {
		free(q);
		return 0;
	}

	q_elem * head = q->head;
	q->head = q->head->next;
	free(head);
	return queue_destroy(q);
}
