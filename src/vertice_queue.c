//
// Created by syvon on 6/19/20.
//

#include "vertice_queue.h"
#include <stdlib.h>

vertice_queue_t *vertice_queue_new(u32 cap) {
    vertice_queue_t *q = malloc(sizeof(vertice_queue_t));

    if (NULL == q) {
        return q;
    }

    q->data = malloc(sizeof(u32) * cap);
    q->b = new_bitset(cap);
    bitset_all(q->b, 0);

    q->lo = 0;
    q->hi = 0;
    q->sz = 0;
    q->cap = cap;

    return q;
}

void vertice_queue_destroy(vertice_queue_t *q) {
    bitset_destroy(q->b);
    free(q->data);
    free(q);
}

u32 vertice_queue_pop(vertice_queue_t *q) {
    u32 e = q->data[q->lo];
    q->lo = (q->lo + 1) % q->cap;
    q->sz--;
    bitset_set(q->b, e, 0);
    return e;
}

void vertice_queue_push(vertice_queue_t *q, u32 e) {
    if (!bitset_set(q->b, e, 1)) {
        q->data[q->hi] = e;
        q->hi = (q->hi + 1) % q->cap;
        q->sz++;
    }
}
