#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "safequeue.h"

struct q_item **pq;
int size = 0;
int max_size;
pthread_mutex_t queue_mutex;
pthread_cond_t queue_worker_cond;

void create_queue(int max_queue_size) {
    max_size = max_queue_size;
    pq = malloc(max_queue_size * sizeof(struct q_item*));
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_worker_cond, NULL);
}

int add_work(int fd, int p, char *path, int delay) {
    struct q_item *new_item = malloc(sizeof(struct q_item));

    new_item->client_fd = fd;
    new_item->priority = p;
    new_item->path = path;
    new_item->delay = delay;

    pthread_mutex_lock(&queue_mutex);

    if (size >= max_size) {
        return -1;
    }

    pq[size] = new_item;
    size++;

    pthread_cond_signal(&queue_worker_cond); // Signal that new work is added
    pthread_mutex_unlock(&queue_mutex);

    return 0;
}

int get_highest_priority() {
    int highest_p = -1;
    int index = -1;

    for (int i = 0; i < size; i++) {
        if (pq[i]->priority > highest_p) {
            highest_p = pq[i]->priority;
            index = i;
        }
    }

    return index;
}

struct q_item *get_work() {
    pthread_mutex_lock(&queue_mutex);

    while (size <= 0) {
        pthread_cond_wait(&queue_worker_cond, &queue_mutex); // Wait for work
    }

    int remove_idx = get_highest_priority();
    struct q_item *item = pq[remove_idx];

    for (int i = remove_idx; i < size - 1; i++) {
        pq[i] = pq[i + 1];
    }

    size--;

    pthread_mutex_unlock(&queue_mutex);

    return item;
}

struct q_item *get_work_nonblocking() {
    pthread_mutex_lock(&queue_mutex);


    if (size <= 0) {
        pthread_mutex_unlock(&queue_mutex);
        return (struct q_item*)0;
    }

    int remove_idx = get_highest_priority();

    struct q_item *item = pq[remove_idx];

    for (int i = remove_idx; i < size - 1; i++) {
        pq[i] = pq[i + 1];
    }
    size--;

    pthread_mutex_unlock(&queue_mutex);

    return item;
}

// Remember to free resources and destroy mutex and condition variable
void destroy_queue() {
    pthread_mutex_lock(&queue_mutex);

    for (int i = 0; i < size; i++) {
        free(pq[i]->path);
        free(pq[i]);
    }

    free(pq);
    size = 0;

    pthread_mutex_unlock(&queue_mutex);
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_worker_cond);
}
