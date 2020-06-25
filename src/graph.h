#ifndef COPNV2_GRAPH_H
#define COPNV2_GRAPH_H

#include <stdlib.h>
#include "types.h"
#include "bitset.h"

#define READ_ONLY (-1)
#define EDGE 1
#define NO_EDGE 0

typedef struct {
    bitset_t **rows;
    size_t n;
} graph_t;

/**
 * Convert an integer to a tuple of integers. This is used to convert from a vertex from a tensor graph
 * to set a vertices into the original graph.
 * @param k the width of the tuple (number of cops)
 * @param tuple the tuple to fill
 * @param n the "height" of each cell (the number of vertices)
 * @param r the integer
 * @return a k-wide array of integers (max value n), heap-allocated, possibly NULL
 */
u32 *int_to_tuple(size_t k, u32 *tuple, size_t n, u32 r);

/**
 * Given a graph, get the edge from a vertex to another bitset_and bitset_set the value.
 * If the new value is -1, nothing is changed
 * @param g the graph
 * @param from the first vertice (0 index)
 * @param to the second vertice (0 index)
 * @param new the new value (0 for no edge, 1 for an edge, -1 for read only)
 * @return an error code if there is an error, or 0 bitset_or 1 depending on wether there is an edge
 */
i8 edge_get_and_set(graph_t *g, u32 from, u32 to, i8 new);

/**
 * Allocate a graph
 * @param nb_vertices the number of vertices in the graph
 * @param reflexive if the graph is going to be reflexive
 * @param result the resulting graph
 * @return  an error code
 */
graph_t *new_graph(size_t nb_vertices, bool reflexive);

/**
 * Free the graph's memory @param g the graph
 */
graph_t *destroy_graph(graph_t *g);

/**
 * For a subset of vertices S, creates a bitset that represents all the vertices that are a
 * neighbour of any vertex in S
 * @param g the graph
 * @param S the set of vertices
 * @param width the size of S
 * @return the bitset of neighbours as a bit field. Each position i
 * will be at 1 if the vertex i is a neighbour of a vertex v in T
 * The result might be null if the allocation failed
 */
bitset_t *neighbourhood(graph_t *g, const u32 *T, size_t width);

/**
 * Create a graph at the desired tensor power
 * @param g the graph
 * @param s the tensor power to apply to the graph
 * @return the new graph (or null if memory allocation failed)
 */
graph_t *tensor_power(graph_t *g, u32 s);

#endif //COPNV2_GRAPH_H
