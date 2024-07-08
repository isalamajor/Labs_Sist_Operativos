#include "../queue.h"
#include <stdio.h>
#define LENGTH 10
int test_1(int q_length) {
    queue * q=queue_init(q_length);
    // The queue should be empty
    if (queue_get(q) != NULL) {
        printf("The queue should be empty!!\n");
        queue_destroy(q);
        return -1;
    }

    int arr[q_length];
    for (int i = 0; i<q_length;i++) {
        arr[i] = i;
        queue_put(q, &arr[i]);
    }
    // By this point the queue should be full
    if (!queue_full(q) || queue_put(q, &arr[0]) != -1) {
        queue_destroy(q);
        printf("The queue should be full!!\n");
        return -1;
    }
    // Now we check that we get the elements in order
    for (int i = 0; i<q_length;i++) {
        int a = *(int *) queue_get(q);
        if (a != arr[i]) {
            printf("Expected: %d. Got: %d\n", arr[i-1], a);
            queue_destroy(q);
            return -1;
        }
    }
    // Again, we will check that the queue does not give us any value when empty
    if (!queue_empty(q) || queue_get(q) != NULL) {
        printf("The queue should be empty!!\n");
        queue_destroy(q);
        return -1;
    }
    // Now we check that the current length is being properly updated
    int objective = q_length/2;
    for (int i = 0; i<objective;i++) {
        queue_put(q, &arr[i]);
    }

    if (q->current_length != objective) {
        printf("Expected: %d. Got: %d\n", objective, q->current_length);
        queue_destroy(q);
        return -1;

    }

    queue_destroy(q);
    return 0;
}

int test_2(int q_length) {
    queue* q = queue_init(q_length);
    int arr[q_length];
    for (int i = 0; i<q_length;i++) {
        arr[i] = i;
        queue_put(q, &arr[i]);
    }
    q_elem* current = q->head;
    printf("Head: ");
    for (int i = 0; i<q_length;i++) {
        int val = *(int *)current->value_ptr;
        printf("%d", val);
        if (i == q_length-1) printf("\n");
        else {
            printf("->");
            current = current->next;
        }
    }
    if (q->tail != current) {
        printf("The tail was not properly reached from the head\n");
        queue_destroy(q);
        return -1;
    }
    printf("Tail: ");
    for (int i = 0; i<q_length;i++) {
        int val = *(int *)current->value_ptr;
        printf("%d", val);
        if (i == q_length-1) printf("\n");
        else {
            printf("->");
            current = current->prev;
        }
    }
    if (q->head != current) {
        printf("The head was not properly reached from the tail\n");
        queue_destroy(q);
        return -1;
    }
    queue_destroy(q);
    return 0;
}

int main(int argc, char *argv[]) {
    if (test_1(LENGTH) != 0) {
        printf("Something went wrong\n");
        return 1;
    }
    if (test_2(LENGTH) != 0) {
        printf("Something went wrong\n");
        return 1;
    }
    printf("Success!!\n");
    return 0;
}
