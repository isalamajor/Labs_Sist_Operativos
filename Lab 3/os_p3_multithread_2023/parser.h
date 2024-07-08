#ifndef MYPARSER
#define MYPARSER

#include "queue.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    create,
    deposit,
    withdraw,
    transfer,
    balance
} operation_type;

typedef struct {
    operation_type type;
    int params[3];
} op_t;

op_t * parse_operation(char *line);
queue * parse_file(const char * filename, int * op_num);

#endif
