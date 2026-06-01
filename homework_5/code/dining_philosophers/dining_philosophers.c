#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define NUM_PHILOSOPHERS 5
#define MAX_CYCLES 3

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];
volatile int running = 1;

typedef struct {
    int id;
    int use_deadlock_fix;
} philosopher_arg_t;

void* philosopher(void* arg) {
    philosopher_arg_t* p_arg = (philosopher_arg_t*)arg;
    int id = p_arg->id;
    int use_fix = p_arg->use_deadlock_fix;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    int cycles = 0;

    while (running && cycles < MAX_CYCLES) {
        printf("Philosopher %d is THINKING...\n", id);
        usleep(100000 + rand() % 200000);

        if (use_fix && id == NUM_PHILOSOPHERS - 1) {
            pthread_mutex_lock(&chopsticks[right]);
            pthread_mutex_lock(&chopsticks[left]);
            printf("  Philosopher %d picked up chopsticks [%d(→), %d(←)]\n",
                   id, right, left);
        } else {
            pthread_mutex_lock(&chopsticks[left]);
            pthread_mutex_lock(&chopsticks[right]);
            printf("  Philosopher %d picked up chopsticks [%d(←), %d(→)]\n",
                   id, left, right);
        }

        printf("Philosopher %d is EATING...  (cycle %d)\n", id, cycles + 1);
        usleep(100000 + rand() % 200000);

        pthread_mutex_unlock(&chopsticks[left]);
        pthread_mutex_unlock(&chopsticks[right]);
        printf("Philosopher %d put down chopsticks\n", id);

        cycles++;
    }
    return NULL;
}

void run_demo(int use_deadlock_fix, const char* label) {
    pthread_t philosophers[NUM_PHILOSOPHERS];

    printf("\n=== %s ===\n\n", label);

    running = 1;
    srand(time(NULL));

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_arg_t* arg = malloc(sizeof(philosopher_arg_t));
        arg->id = i;
        arg->use_deadlock_fix = use_deadlock_fix;
        pthread_create(&philosophers[i], NULL, philosopher, arg);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }

    printf("\nDemo completed: %s\n", label);
}

int main() {
    printf("========================================\n");
    printf("  Dining Philosophers Problem\n");
    printf("  %d philosophers, %d chopsticks\n",
           NUM_PHILOSOPHERS, NUM_PHILOSOPHERS);
    printf("========================================\n");

    run_demo(0, "Without deadlock fix (prone to deadlock!)");

    printf("\n----------------------------------------\n");

    run_demo(1, "With deadlock fix (asymmetric pickup)");

    printf("\nAll demos completed!\n");
    return 0;
}
