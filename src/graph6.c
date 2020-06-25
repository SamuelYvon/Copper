//
// Created by syvon on 6/18/20.
//

#include "graph6.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Get the graph size from a g6 string graph, where
 * all bytes have been -63
 * @param data_string the data string
 * @return the len of the graph
 */
u32 g6_len(u8 *data_string, size_t *start) {
    if (data_string[0] <= 62) {
        *start = 1;
        return data_string[0];
    } else if (data_string[1] <= 62) {
        *start = 4;
        return (data_string[1] << 12) + (data_string[2] << 6) + data_string[3];
    } else {
        *start = 8;
        return (data_string[2] << 30) +
               (data_string[3] << 24) +
               (data_string[4] << 18) +
               (data_string[5] << 12) +
               (data_string[6] << 6) +
               data_string[7];
    }
}

/**
 *
 * @param raw_data
 * @return
 */
graph_t *from_g6(char *raw_data) {
    size_t bytes = strlen(raw_data);
    u8 *_raw_data = (u8 *) malloc(sizeof(char) * bytes);
    memcpy(_raw_data, raw_data, bytes);

    for (size_t b = 0; b < bytes; ++b) {
        _raw_data[b] -= 63U;
    }

    size_t start = 0;
    u32 n = g6_len(_raw_data, &start);

    bitset_t *edge_bits = new_bitset(((n * (n - 1)) / 2));

    u32 cursor = 0;
    for (uint scout = start; scout < bytes; ++scout) {
        for (i8 rank = 5; rank >= 0; --rank) {
            bitset_set(edge_bits, cursor++, (_raw_data[scout] >> rank) & 1U);
        }
    }

    graph_t *g = new_graph(n, 1);

    cursor = 0;
    for (u32 j = 1; j < n; ++j) {
        for (u32 i = 0; i < j; ++i) {
            edge_get_and_set(g, i, j, bitset_set(edge_bits, cursor++, -1));
        }
    }

    bitset_destroy(edge_bits);
    free(_raw_data);

    return g;
}
