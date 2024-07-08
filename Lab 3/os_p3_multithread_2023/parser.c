#include "parser.h"
#include "queue.h"
#include <stdio.h>

#define MAX_OP_NUM 200
#define MAX_LINE_LENGTH 40
#define MAX_OP_LENGTH 10

op_t *parse_operation(char *line) {
    int param0 = 0, param1 = -1, param2 = -1;
    char op_type[MAX_OP_LENGTH];
    sscanf(line, "%s %d %d %d\n", op_type, &param0, &param1, &param2);
    op_t *op = (op_t *)malloc(sizeof(op_t));
    if (op == NULL) return NULL;

    if (!strncmp(op_type, "CREATE", MAX_OP_LENGTH)) {
        op->type = create;
    } else if (!strncmp(op_type, "DEPOSIT", MAX_OP_LENGTH)) {
        op->type = deposit;
    } else if (!strncmp(op_type, "WITHDRAW", MAX_OP_LENGTH)) {
        op->type = withdraw;
    } else if (!strncmp(op_type, "BALANCE", MAX_OP_LENGTH)) {
        op->type = balance;
    } else if (!strncmp(op_type, "TRANSFER", MAX_OP_LENGTH)) {
        op->type = transfer;
    } else {
       free(op);
       return NULL;
    }

    op->params[0] = param0;
    op->params[1] = param1;
    op->params[2] = param2;
    return op;
}

queue *parse_file(const char *filename, int *op_num) {
    // Opening the file where the operations are stored
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
      printf("Error opening %s\n", filename);
    }

    char line[MAX_LINE_LENGTH];
    fgets(line, MAX_LINE_LENGTH, file);

    // Check length < max_length
    int max_operations;
    sscanf(line, "%d", &max_operations);
    if (max_operations > MAX_OP_NUM) {
        fclose(file);
        return NULL;
    }

    // Initialize the queue
    queue *q = queue_init(max_operations);

    int count = 0;
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        if (count > max_operations) {
           fclose(file);
           queue_destroy(q);
           return NULL;
        }
        op_t *op = parse_operation(line);
        if (op == NULL) {
            fclose(file);
            queue_destroy(q);
            return NULL;
        }
        queue_put(q, op);
        ++count;
    }
    if (count < max_operations) {
        fclose(file);
        queue_destroy(q);
        return NULL;
    }
    fclose(file);
    *op_num = count;
    return q;
};
