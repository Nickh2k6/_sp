#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 10

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0, out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void* producer(void* arg) {
    int id = *(int*)arg;
    free(arg);

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * 1000 + i;

        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {
            printf("  Producer %d: buffer full, waiting...\n", id);
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[in] = item;
        printf("Producer %d produced: %-4d  [buffer: %d/%d]\n",
               id, item, count + 1, BUFFER_SIZE);
        in = (in + 1) % BUFFER_SIZE;
        count++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        usleep(50000 + rand() % 100000);
    }
    return NULL;
}

void* consumer(void* arg) {
    int id = *(int*)arg;
    free(arg);
    int total_items = ITEMS_PER_PRODUCER * NUM_PRODUCERS / NUM_CONSUMERS;

    for (int i = 0; i < total_items; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            printf("  Consumer %d: buffer empty, waiting...\n", id);
            pthread_cond_wait(&not_empty, &mutex);
        }

        int item = buffer[out];
        printf("Consumer %d consumed: %-4d  [buffer: %d/%d]\n",
               id, item, count - 1, BUFFER_SIZE);
        out = (out + 1) % BUFFER_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        usleep(80000 + rand() % 150000);
    }
    return NULL;
}

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    printf("=== Producer-Consumer Problem ===\n");
    printf("Buffer size: %d\n", BUFFER_SIZE);
    printf("Producers: %d, each producing %d items\n",
           NUM_PRODUCERS, ITEMS_PER_PRODUCER);
    printf("Consumers: %d\n\n", NUM_CONSUMERS);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&producers[i], NULL, producer, id);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&consumers[i], NULL, consumer, id);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    printf("\nAll items produced and consumed successfully!\n");
    return 0;
}
