#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proxyserver.h"

struct q_item {
    struct http_request *req;
    int priority;
};

struct q_item *pq;
int size = 0;

void create_queue(int max_queue_size) {
    pq = malloc(max_queue_size * sizeof(struct q_item));
}

void add_work(struct q_item *i) {
    pq[size] = *i;
    size++;

    // condition signal
}

int get_highest_priority() {
    int highest_p = -1; // maybe make this lower
    int index;

    for (int i = 0; i < size; i++) {
        if (pq[i].priority > highest_p) {
            highest_p = pq[i].priority;
            index = i;
        }
    }

    return index;
}

struct http_request *get_work() {

    while (size <= 0) {
        // condition wait
    }

    int remove_idx = get_highest_priority();
    struct http_request *req = pq[remove_idx].req;

    for (int i = remove_idx; i < size - 1; i++) {
        pq[i] = pq[i + 1];
    }

    size--;

    return req;
}