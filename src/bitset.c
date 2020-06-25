#include "bitset.h"
#include <stdlib.h>

bitset_t *new_bitset(u32 bits) {
    u32 no_of_blocks = (bits / BITSET_WIDTH) + ((bits % BITSET_WIDTH) > 0);
    bitset_t *field;
    if (NULL != (field = malloc(sizeof(bitset_t)))) {
        if (NULL != (field->parts = malloc(sizeof(BITSET_DATA_UNIT) * no_of_blocks))) {
            field->l = no_of_blocks;
            field->bits = bits;
            for (u32 i = 0; i < no_of_blocks; ++i) {
                field->parts[i] = 0x00;
            }
        } else {
            free(field);
            field = NULL;
        }
    }

    return field;
}

bitset_t *bitset_clone(bitset_t *left) {
    bitset_t *new = new_bitset(left->bits);

    for (u32 i = 0; i < left->l; ++i) {
        new->parts[i] = left->parts[i];
    }

    return new;
}

bitset_t *bitset_destroy(bitset_t *left) {
    if (NULL != left) {
        if (NULL != left->parts) {
            free(left->parts);
        }
        free(left);
    }

    return NULL;
}

bool bitset_or(bitset_t *left, bitset_t *right) {
    bool change = 0;
    u32 l = left->l;
    BITSET_DATA_UNIT *a = left->parts;
    BITSET_DATA_UNIT *b = right->parts;
    for (u32 i = 0; i < l; ++i) {
        BITSET_DATA_UNIT old = a[i];
        a[i] |= b[i];
        change |= (old != a[i]);
    }
    return change;
}

bool bitset_and(bitset_t *left, bitset_t *right) {
    bool change = 0U;
    u32 l = left->l;
    BITSET_DATA_UNIT *a = left->parts;
    BITSET_DATA_UNIT *b = right->parts;
    for (u32 i = 0; i < l; ++i) {
        BITSET_DATA_UNIT old = a[i];
        a[i] &= b[i];
        change |= (a[i] != old);
    }
    return change;
}

bitset_t *bitset_not(bitset_t *left, bitset_t *right) {
    u32 l = right->l;
    BITSET_DATA_UNIT *a = left->parts;
    BITSET_DATA_UNIT *b = right->parts;
    for (u32 i = 0; i < l; ++i) {
        a[i] = ~b[i];
    }
    return left;
}

u8 bitset_set(bitset_t *left, u32 bit, i8 val) {
    u32 addr = bit / BITSET_WIDTH;
    u8 offset = bit % BITSET_WIDTH;
    BITSET_DATA_UNIT block = left->parts[addr];
    u8 previous = (block & (1U << offset)) > 0;

    if (0 == val) {
        left->parts[addr] &= ~(1U << offset);
    } else if (1 == val) {
        left->parts[addr] |= (1U << offset);
    }

    return previous;
}

u32 *bitset_indices(bitset_t *b, u32 *vertex_count) {
    u32 sz = 0;
    u32 bits = b->bits;

    u32 *s = malloc(sizeof(u32) * bits);
    if (!s) { return s; }

    u32 index = 0;
    u32 i = 0;
    while (i < b->l && index < bits) {
        BITSET_DATA_UNIT block = b->parts[i++];
        if (block > 0) {
            for (u8 j = 0; j < BITSET_WIDTH && (index + j) < bits; ++j) {
                if (block & (1U << j)) {
                    s[sz++] = index + j;
                }
            }
        }

        index += BITSET_WIDTH;
    }

    *vertex_count = sz;

    return s;
}

u32 bitset_sum(bitset_t *b) {
    u32 s = 0;

    for (u32 i = 0; i < b->l; ++i) {
        s += b->parts[i];
    }

    return s;
}

bool bitset_any(bitset_t *b) {
    bool any = FALSE;
    for (u32 i = 0; i < b->l; ++i) {
        if (b->parts[i] > 0) {
            any = TRUE;
            break;
        }
    }
    return any;
}

void bitset_all(bitset_t *b, bool v) {
    for (u32 i = 0; i < b->bits; ++i) {
        bitset_set(b, i, v);
    }
}
