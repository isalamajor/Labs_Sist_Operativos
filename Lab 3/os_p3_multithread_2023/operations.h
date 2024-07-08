#ifndef MYOPERATIONS
#define MYOPERATIONS

#include "parser.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    int balance;
    bool active;
} acc;

typedef struct {
    int max_accounts;
    acc* accounts[];
} bank;

int operate_bank(const op_t* operation, bank * bank, int numop, int * global_balance);
bank * bank_init(int max_accounts);
int bank_destroy(bank *bank);
#endif
