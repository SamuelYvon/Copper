//
// Created by syvon on 6/18/20.
//

#ifndef COPNV2_GRAPH6_H
#define COPNV2_GRAPH6_H

#define G6_HEADER ">>graph6<<"
#define G6_HEADER_LEN 10

#include <stdlib.h>
#include "types.h"
#include "graph.h"

/**
 * Get the graph size from a g6 string graph, where
 * all bytes have been -63
 * @param data_string the data string
 * @return the len of the graph
 */
u32 g6_len(u8 *data_string, size_t *start);

/**
 *
 * @param raw_data
 * @return
 */
graph_t *from_g6(char *raw_data);

#endif //COPNV2_GRAPH6_H
