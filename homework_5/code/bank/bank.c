#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_OPERATIONS 100000

int balance = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* deposit_without_lock(void* arg) {
    (void)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        balance++;
    }
    return NULL;
}

void* withdraw_without_lock(void* arg) {
    (void)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        balance--;
    }
    return NULL;
}

void* deposit_with_lock(void* arg) {
    (void)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        balance++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* withdraw_with_lock(void* arg) {
    (void)arg;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        balance--;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void run_test(
    const char* label,
    void* (*deposit_fn)(void*),
    void* (*withdraw_fn)(void*)
) {
    balance = 0;
    pthread_t t1, t2;

    printf("=== %s ===\n", label);

    pthread_create(&t1, NULL, deposit_fn, NULL);
    pthread_create(&t2, NULL, withdraw_fn, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final balance: %d (expected: 0)\n", balance);
    if (balance == 0) {
        printf("Result: CORRECT\n");
    } else {
        printf("Result: WRONG - race condition caused data loss!\n");
    }
}

int main() {
    printf("=== Bank Account Simulation ===\n");
    printf("Depositing 100,000 times & withdrawing 100,000 times\n");
    printf("Initial balance: 0, Expected final balance: 0\n\n");

    run_test("Without mutex (race condition)",
             deposit_without_lock, withdraw_without_lock);
    printf("\n");
    run_test("With mutex (safe synchronization)",
             deposit_with_lock, withdraw_with_lock);

    return 0;
}
