#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

struct q_item {
    int client_fd;
    int priority;
};

struct q_item *pq;
int size = 0;
int max_size;
pthread_mutex_t queue_mutex;
pthread_cond_t queue_cond;

void create_queue(int max_queue_size) {
    max_size = max_queue_size;
    pq = malloc(max_queue_size * sizeof(struct q_item));
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_cond, NULL);
}

void add_work(struct q_item *i) {
    pthread_mutex_lock(&queue_mutex);

    while (size >= max_size) {
        // Wait until there is space in the queue
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    pq[size] = *i;
    size++;

    pthread_cond_signal(&queue_cond); // Signal that new work is added
    pthread_mutex_unlock(&queue_mutex);
}

int get_highest_priority() {
    int highest_p = -1;
    int index = -1;

    for (int i = 0; i < size; i++) {
        if (pq[i].priority > highest_p) {
            highest_p = pq[i].priority;
            index = i;
        }
    }

    return index;
}

int get_work() {
    pthread_mutex_lock(&queue_mutex);

    while (size <= 0) {
        pthread_cond_wait(&queue_cond, &queue_mutex); // Wait for work
    }

    int remove_idx = get_highest_priority();
    int client_fd = pq[remove_idx].client_fd;

    for (int i = remove_idx; i < size - 1; i++) {
        pq[i] = pq[i + 1];
    }

    size--;

    pthread_cond_signal(&queue_cond); // Signal that work has been removed
    pthread_mutex_unlock(&queue_mutex);

    return client_fd;
}

int get_work_nonblocking() {
    pthread_mutex_lock(&queue_mutex);

    int remove_idx = get_highest_priority();
    int client_fd = pq[remove_idx].client_fd;

    pthread_mutex_unlock(&queue_mutex);

    return client_fd;
}

// Remember to free resources and destroy mutex and condition variable
void destroy_queue() {
    free(pq);
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_cond);
}
