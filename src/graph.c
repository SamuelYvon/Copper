#include "graph.h"
#include "bitset.h"

/**
 * Compute the integer power
 * Based off the implementation given at
 * https://stackoverflow.com/a/101613/5092307
 * @param a the number to compute a power of
 * @param e the exponent to put the number too
 * @return a^e (int)
 */
u32 ipow(u32 a, u32 e) {
    u32 r = 1;

    while (1) {
        // odd?
        if (e & 1u) {
            r *= a;
        }
        // Always even here
        e = e >> 1;
        // Unless e is 1 or 0, square the base
        if (0 == e) {
            break;
        }
        a = a * a;
    }

    return r;
}

u32 *int_to_tuple(size_t k, u32 *tuple, size_t n, u32 r) {
    for (size_t i = 0; i < k; ++i) {
        if (r > 0) {
            u32 d = ipow(n, (k - i - 1));
            tuple[i] = r / d;
            r -= tuple[i] * d;
        } else {
            tuple[i] = 0;
        }
    }

    return tuple;
}

i8 edge_get_and_set(graph_t *g, u32 from, u32 to, i8 new) {

    if (NULL == g) {
        return -1;
    }

    size_t n = g->n;
    bitset_t **rows = g->rows;

    if (from >= n || to >= n) {
        return -1;
    }

    bitset_t *a = rows[from];

    u8 old = bitset_set(a, to, -1);

    if (new > READ_ONLY) {
        bitset_t *b = rows[to];
        bitset_set(a, to, new);
        bitset_set(b, from, new);
    }

    return old;
}

/**
 * Allocate a graph
 * @param nb_vertices the number of vertices in the graph
 * @param reflexive if the graph is going to be reflexive
 * @param result the resulting graph
 * @return the graph (or null, if allocation failed)
 */
graph_t *new_graph(size_t nb_vertices, bool reflexive) {
    graph_t *g = (graph_t *) malloc(sizeof(graph_t));

    if (!g) {
        return NULL;
    }

    g->n = nb_vertices;
    g->rows = malloc(sizeof(bitset_t *) * nb_vertices);

    if (!g->rows) {
        free(g);
        return NULL;
    }

    // Clear to null so we can deallocate properly if it fails
    for (u32 i = 0; i < nb_vertices; ++i) {
        g->rows[i] = NULL;
    }

    for (u32 i = 0; i < nb_vertices; ++i) {
        if (NULL == (g->rows[i] = new_bitset(nb_vertices))) {
            // Cleanup
            return destroy_graph(g);
        } else {
            bitset_all(g->rows[i], FALSE);
        }
    }

    if (reflexive) {
        // Set the diagonal
        for (size_t i = 0; i < nb_vertices; ++i) {
            edge_get_and_set(g, i, i, EDGE);
        }
    }

    return g;
}

/**
 * Free the graph's memory @param g the graph
 */
graph_t *destroy_graph(graph_t *g) {
    for (u32 i = 0; i < g->n; ++i) {
        if (NULL != g->rows[i]) {
            bitset_destroy(g->rows[i]);
        }
    }
    free(g->rows);
    free(g);
    return NULL;
}

bitset_t *neighbourhood(graph_t *g, const u32 *S, size_t width) {
    bitset_t *b = new_bitset(g->n);

    if (!b) { return b; }

    for (u32 i = 0; i < width; ++i) {
        bitset_or(b, g->rows[S[i]]);
    }

    return b;
}

graph_t *tensor_power(graph_t *g, u32 s) {
    size_t n = g->n;
    size_t N = (size_t) ipow(n, s);

    graph_t *tensor_graph = new_graph(N, TRUE);

    if (!tensor_graph) { return tensor_graph; }

    u32 *cached_a, *cached_b;
    cached_a = (u32 *) malloc(sizeof(u32) * s);
    cached_b = (u32 *) malloc(sizeof(u32) * s);

    for (u32 i = 0; i < N; ++i) {
        for (u32 j = i; j < N; ++j) {
            // We get a potential edge (i,j);

            bool edge = 1;
            if (i != j) {
                // Both i and j are encoded tuples. We name their tuples a, b respectively
                u32 *a = int_to_tuple(s, cached_a, n, i);
                u32 *b = int_to_tuple(s, cached_b, n, j);

                // There is an edge between a bitset b iif all the respective components are adjacent
                for (u32 c = 0; c < s && edge; ++c) {
                    edge = edge_get_and_set(g, a[c], b[c], READ_ONLY);
                }
            }

            edge_get_and_set(tensor_graph, i, j, edge);
        }
    }

    // Get rid of the tuples
    free(cached_a);
    free(cached_b);

    return tensor_graph;
}


bool graph_has_pitfall(graph_t *g, u8 k) {

    bool has_pit = FALSE;

    u32 *indices_set = malloc(sizeof(u32) * k);
    for (u32 u = 0; u < g->n && !has_pit; ++u) {
        u32 neigh_sz = 0;
        u32 *neighs = bitset_indices(g->rows[u], &neigh_sz);
        // It could be less than k; so in theory we need to check
        // all subsets, but technically, we do not, since we do an OR of
        // the neighbour sets.
        bitset_t *covered = new_bitset(g->rows[u]->bits);

        u32 loops = ipow(neigh_sz, k);

        for (u32 i = 0; i < loops; ++i) {
            bitset_t *n = bitset_clone(g->rows[u]);
            bitset_all(covered, 0);
            int_to_tuple(k, indices_set, neigh_sz, i);

            for (u32 j = 0; j < k; ++j) {
                u32 neigh_index = indices_set[j];
                u32 neigh = neighs[neigh_index];

                if (neigh == u) {
                    // of course they will have the same
                    // neighbourhood
                    continue;
                }

                bitset_or(covered, g->rows[neigh]);
            }

            bitset_and(n, covered);
            has_pit = bitset_eqs(g->rows[u], n);
            bitset_destroy(n);

            if (has_pit) {
                break;
            }
        }

        bitset_destroy(covered);
    }

    free(indices_set);
    return has_pit;
}
