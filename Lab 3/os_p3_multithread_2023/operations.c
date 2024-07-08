#include "operations.h"
#include <stdbool.h>
/* #define WRONG_ACCOUNT(account, bank) if (account >= bank->max_accounts) \ */
/*         printf("Invalid account: there are %d accounts, this is the #%d\n", bank->max_accounts, account) */
/* #define UNINITIALIZED_ACCOUNT(account, bank) if (!bank->accounts[account].active) \ */
/*         printf("The account #%d has not been created yet. Omitting operation...\n", account) */
/* #define NEGATIVE_AMOUNT(amount) if (amount < 0) \ */
/*         printf("Invalid amount, it must be positive\n") */

// TODO Error checking
int operate_bank(const op_t* operation, bank * bank, int numop, int * global_balance) {
    int account = operation->params[0]; // Because valid accounts start at 1
    if (account > bank->max_accounts) {
        printf("Invalid account: there are %d accounts, this is the #%d\n", bank->max_accounts, account);
        return 1;
    }
    int amount1 = operation->params[1];
    /* if (amount1 < 0) { */
    /*     printf("Invalid amount, it must be non-negative\n"); */
    /*     return 1; */
    /* } */
    int destination = operation->params[1];
    int amount2 = operation->params[2];
    /* if (amount2 < 0) { */
    /*     printf("Invalid amount, it must be non-negative\n"); */
    /*     return 1; */
    /* } */
    /* if (!bank->accounts[account]->active) { */
    /*     printf("The account #%d has not been created yet. Omitting operation...\n", account); */
    /*     return 1; */
    /* } */
    acc * bk_account = bank->accounts[account-1];
    acc * bk_destination = bank->accounts[destination-1];
    switch (operation->type) {
        case create:
            bk_account->active = true;
            printf("%d CREATE %d BALANCE=%d TOTAL=%d\n",
                   numop, account, bk_account->balance, *global_balance);
            break;
        case deposit:
            bk_account->balance += amount1;
            *global_balance += amount1;
            printf("%d DEPOSIT %d %d BALANCE=%d TOTAL=%d\n",
                   numop, account, amount1, bk_account->balance, *global_balance);
            break;
        case withdraw:
            bk_account->balance -= amount1;
            *global_balance -= amount1;
            printf("%d WITHDRAW %d %d BALANCE=%d TOTAL=%d\n",
                   numop, account, amount1, bk_account->balance, *global_balance);
            break;
        case transfer:
            /* if (destination >= bank->max_accounts) { */
            /*     printf("Invalid account: there are %d accounts, this is the #%d\n", bank->max_accounts, account); */
            /*     return -1; */
            /* } */
            /* if (!bank->accounts[destination]->active) { */
            /*     printf("The account #%d has not been created yet. Omitting operation...\n", account); */
            /*     return -1; */
            /* } */
            bk_account->balance -= amount2;
            bk_destination->balance += amount2;
            printf("%d TRANSFER %d %d %d BALANCE=%d TOTAL=%d\n",
                   numop, account, destination, amount2, bk_destination->balance, *global_balance);
            break;
        case balance:
            printf("%d BALANCE %d BALANCE=%d TOTAL=%d\n",
                   numop, account, bk_account->balance, *global_balance);
            break;
        default:
            printf("INVALID OPERATION: %d\n", operation->type);
            return 1;
    }
    return 0;
}

bank *bank_init(int max_accounts) {
    bank * b = malloc(sizeof(b[0]) * max_accounts + sizeof(bank));
    b->max_accounts = max_accounts;
    for (int i = 0; i<max_accounts; i++) {
        acc * account = malloc(sizeof(acc));
        account->balance = 0;
        account->active = false;
        b->accounts[i] = account;
    }

    return b;
}

int bank_destroy(bank *bank) {
    for (int i = 0; i<bank->max_accounts; i++) {
        free(bank->accounts[i]);
    }
    free(bank);
    return 0;
}
