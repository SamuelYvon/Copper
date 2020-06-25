//
// Created by syvon on 6/19/20.
//

#ifndef COPNV2_VERTICE_QUEUE_H
#define COPNV2_VERTICE_QUEUE_H

#include "types.h"
#include "bitset.h"

typedef struct {
    u32 lo, hi, sz, cap;
    u32 *data;
    bitset_t *b;
} vertice_queue_t;

vertice_queue_t *vertice_queue_new(u32 cap);

void vertice_queue_destroy(vertice_queue_t *q);

u32 vertice_queue_pop(vertice_queue_t *q);

void vertice_queue_push(vertice_queue_t *q, u32 e);

#endif //COPNV2_VERTICE_QUEUE_H
