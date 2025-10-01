//
// Created by Adeel Ahmad on 2/10/25.
//

#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

typedef struct queue {
    pthread_mutex_t mutex;
    pthread_cond_t added;
    pthread_cond_t removed;
    void **q;
    int size;
    int data;
    int in;
    int out;
} queue_t;

queue_t *queue_new(int size) {
    queue_t *q = calloc(1, sizeof(queue_t));
    q->size = size;
    q->q = calloc(size, sizeof(void *));
    pthread_mutex_init(&(q->mutex), 0);
    pthread_cond_init(&(q->added), 0);
    pthread_cond_init(&(q->removed), 0);
    return q;
}

void queue_delete(queue_t **q) {
    free((*q)->q);
    (*q)->q = NULL;
    pthread_mutex_destroy(&((*q)->mutex));
    pthread_cond_destroy(&((*q)->added));
    pthread_cond_destroy(&((*q)->removed));
    free(*q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) {
    pthread_mutex_lock(&q->mutex);

    while (q->data >= q->size) {
        pthread_cond_wait(&q->removed, &q->mutex);
    }

    (q->q)[q->in] = elem;
    q->in++;
    q->in = q->in % q->size;
    q->data++;

    pthread_mutex_unlock(&q->mutex);
    pthread_cond_signal(&q->added);
    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    pthread_mutex_lock(&q->mutex);

    while (q->data <= 0) {
        pthread_cond_wait(&q->added, &q->mutex);
    }

    *elem = (q->q)[q->out];
    q->out++;
    q->out = q->out % q->size;
    q->data--;

    pthread_mutex_unlock(&q->mutex);
    pthread_cond_signal(&q->removed);
    return true;
}
