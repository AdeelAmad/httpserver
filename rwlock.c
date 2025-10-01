//
// Created by Adeel Ahmad on 2/10/25.
//

#include "rwlock.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct rwlock {
    int readers_waiting;
    int writers_waiting;
    uint32_t readers_active;
    uint32_t writers_active;
    pthread_cond_t reader_finished;
    pthread_cond_t writer_finished;
    pthread_mutex_t lock;
    PRIORITY p;
    uint32_t n;
    uint32_t reads_n;
} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *lock = calloc(1, sizeof(rwlock_t));
    lock->n = n;
    lock->p = p;
    pthread_cond_init(&lock->reader_finished, 0);
    pthread_cond_init(&lock->writer_finished, 0);
    pthread_mutex_init(&lock->lock, 0);
    return lock;
}

void rwlock_delete(rwlock_t **rw) {
    pthread_mutex_destroy(&(*rw)->lock);
    pthread_cond_destroy(&(*rw)->writer_finished);
    pthread_cond_destroy(&(*rw)->reader_finished);
    free(*rw);
    *rw = NULL;
}

void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->readers_waiting++;
    while (rw->readers_active >= rw->n || (rw->writers_waiting && rw->p == WRITERS)
           || (rw->reads_n >= rw->n && rw->writers_waiting && rw->p == N_WAY) || rw->writers_active) {
        pthread_cond_wait(&rw->reader_finished, &rw->lock);
    }
    rw->readers_waiting--;
    rw->readers_active++;
    rw->reads_n++;
    pthread_mutex_unlock(&rw->lock);
}

void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->readers_active--;
    if (rw->p == READERS) {
        if (rw->readers_waiting) {
            pthread_cond_signal(&rw->reader_finished);
        } else if (rw->writers_waiting && rw->readers_active == 0) {
            pthread_cond_signal(&rw->writer_finished);
        }
    }
    if (rw->p == WRITERS) {
        if (rw->writers_waiting) {
            pthread_cond_signal(&rw->writer_finished);
        } else if (rw->readers_waiting) {
            pthread_cond_signal(&rw->reader_finished);
        }
    }
    if (rw->p == N_WAY) {
        if ((rw->writers_waiting && rw->reads_n >= rw->n) || !rw->readers_waiting) {
            pthread_cond_signal(&rw->writer_finished);
        } else {
            pthread_cond_signal(&rw->reader_finished);
        }
    }
    pthread_mutex_unlock(&rw->lock);
}

void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->writers_waiting++;
    while (rw->writers_active >= 1 || rw->readers_active >= 1
           || (rw->readers_waiting && rw->p == N_WAY && rw->reads_n < rw->n)) {
        pthread_cond_wait(&rw->writer_finished, &rw->lock);
    }
    rw->writers_waiting--;
    rw->writers_active++;
    pthread_mutex_unlock(&rw->lock);
}

void writer_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->writers_active--;
    if (rw->p == READERS) {
        if (rw->readers_waiting) {
            pthread_cond_signal(&rw->reader_finished);
        } else if (rw->writers_waiting) {
            pthread_cond_signal(&rw->writer_finished);
        }
    }
    if (rw->p == WRITERS) {
        if (rw->writers_waiting) {
            pthread_cond_signal(&rw->writer_finished);
        } else if (rw->readers_waiting) {
            pthread_cond_signal(&rw->reader_finished);
        }
    }

    if (rw->p == N_WAY) {
        if (rw->writers_waiting && !rw->readers_waiting) {
            pthread_cond_signal(&rw->writer_finished);
        } else {
            rw->reads_n = 0;
            pthread_cond_signal(&rw->reader_finished);
        }
    }
    pthread_mutex_unlock(&rw->lock);
}
