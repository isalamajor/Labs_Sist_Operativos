//OS-P3 2022-2023
#include "operations.h"
#include "parser.h"
#include "queue.h"

#include <bits/pthreadtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHECK_STRTOL_ERRORS(val) errno != 0 || val < 0
#define MAX_OPERATIONS 200
#define MAX_ARG_LENGTH 10

pthread_mutex_t mutex;
/* pthread_mutex_t prod_mutex; */
/* pthread_mutex_t cons_mutex; */
pthread_cond_t no_empty;
pthread_cond_t no_full;

int operations;
int client_numop = 0;
int bank_numop = 0;
int global_balance = 0;
queue * list_client_ops;
queue * circular_queue;


void *atm(void * args) {
    while (1) {
        pthread_mutex_lock(&mutex);
        if (queue_empty(list_client_ops)) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        ++client_numop;

        while (queue_full(circular_queue))
            pthread_cond_wait(&no_full, &mutex);

        op_t * operation = queue_get(list_client_ops);
        if (!operation) {
            printf("Something went wrong trying to get the bank operation");
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
        }
        queue_put(circular_queue, operation);

        pthread_cond_signal(&no_empty);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void *worker(void * args) {
    bank * balances = args;

    while (1) {
        pthread_mutex_lock(&mutex);
        ++bank_numop;
        int current_op_num = bank_numop;
        if (current_op_num > operations) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        op_t * elem = NULL;
        while (queue_empty(circular_queue))
            pthread_cond_wait(&no_empty, &mutex);
        // If reached this point, the queue should not be empty. If we get a null pointer, it is
        // because something must have gone wrong
        elem = queue_get(circular_queue);
        if (!elem) {
            printf("Something went wrong trying to get the bank operation");
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
        }

        operate_bank(elem, balances, current_op_num, &global_balance);
        free(elem);
        pthread_cond_signal(&no_full);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

int main (int argc, const char * argv[]) {
    if (argc != 6) {
        puts("Invalid number of arguments");
        return EXIT_FAILURE;
    }
    // Parsing the arguments
    errno = 0;
    int buff_size = (int)strtol(argv[1], NULL, 10);
    if (CHECK_STRTOL_ERRORS(buff_size)) {
        perror("Something went wrong: ");
        return 1;
    }

    int num_atms = (int)strtol(argv[2], NULL, 10);
    if (CHECK_STRTOL_ERRORS(num_atms)) {
        perror("Something went wrong: ");
        return 1;
    }

    int num_workers = (int)strtol(argv[3], NULL, 10);
    if (CHECK_STRTOL_ERRORS(num_workers)) {
        perror("Something went wrong: ");
        return 1;
    }

    int max_accounts = (int)strtol(argv[4], NULL, 10);
    if (CHECK_STRTOL_ERRORS(max_accounts)) {
        perror("Something went wrong: ");
        return 1;
    }

    // Create the list with the account balances
    bank *balances = bank_init(max_accounts);

    list_client_ops = parse_file(argv[1], &operations);
    if (list_client_ops == NULL)
        return EXIT_FAILURE;

    circular_queue = queue_init(MAX_OPERATIONS);
    if (circular_queue == NULL)
        return EXIT_FAILURE;

    // Initializing the mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&no_empty, NULL);
    pthread_cond_init(&no_full, NULL);

    // Creating the threads
    pthread_t prod_threads[num_atms];
    pthread_t cons_threads[num_workers];

    for (int i = 0; i<num_atms; i++) {
        pthread_create(&prod_threads[i], NULL, atm, NULL);
    }
    for (int i = 0; i<num_workers; i++) {
        pthread_create(&cons_threads[i], NULL, worker, balances);
    }
    // Joining the threads
    for (int i = 0; i<num_atms; i++) {
        pthread_join(prod_threads[i], NULL);
    }
    for (int i = 0; i<num_workers; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    // Cleaning
    bank_destroy(balances);
    queue_destroy(circular_queue);
    queue_destroy(list_client_ops);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_full);
    pthread_cond_destroy(&no_empty);
    return 0;
}
