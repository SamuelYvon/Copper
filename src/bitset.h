#ifndef COPNV2_BITFIELD_H

#include "types.h"

/* These definition allow the width of a block within a bitset
 * to be changed. In theory, wider blocks should be faster since
 * they require fewer iterations to perform operation on and still require
 * the same amount of CPU cycles to perform. In practice, I can't see
 * a difference.
 */
#define BITSET_DATA_UNIT u32
#define BITSET_WIDTH 32

/**
 * The bitset structure keeps a vector (maybe it should be called a bitvector...)
 * of bits that can be accessed individually. It is used to represent a subset of
 * a universe set by keeping track of which elements are in the set or not.
 */
typedef struct {
    BITSET_DATA_UNIT *parts;
    u32 l;
    u32 bits;
} bitset_t;

/**
 * Create a bitset, return null if failure to get the required memory
 * @param bits the number of bits in the set (size of the universe set)
 * @return pointer to allocated structure
 */
bitset_t *new_bitset(u32 bits);

/**
 * Logical 'or' the right bitfield into the left bitfield.
 * Corresponds to the union of two sets:
 * L <- L \cup R
 * @param left L
 * @param right R
 * @returns whether the left set was mutated
 */
bool bitset_or(bitset_t *left, bitset_t *right);

/**
 * Logical 'and' of the right bitset into the left bitset.
 * Correspond to the intersection of two sets:
 * L <- L \cap R
 * @param left L
 * @param right R
 * @returns whether the right set was mutated
 */
bool bitset_and(bitset_t *left, bitset_t *right);


/**
 * Clone the bitset. It returns an exact copy at a different memory location.
 * (or null if failure to do so)
 * @param left the bitset to copy
 * @return a bitset clone
 */
bitset_t *bitset_clone(bitset_t *left);

/**
 * Perform a logical not on the bitset. This corresponds to taking
 * the complement of the set. The result is stored in left:
 * L <- U \setminus R
 * @param left L
 * @param left R
 * @returns the left set
 */
bitset_t *bitset_not(bitset_t *left, bitset_t *right);

/**
 * Accessor and mutator for a specific bit in a bitset
 * @param left the bitfield
 * @param bit the bit to access or set
 * @param val -1 for no change, 0 or 1 as the value to set
 * @return the value previously affected to the bit
 */
u8 bitset_set(bitset_t *left, u32 bit, i8 val);

/**
 * Destroy the bitfield bitset_and free it
 * @param left
 * @return Return a null ptr
 */
bitset_t *bitset_destroy(bitset_t *left);

/**
 * Compute the list of indices that are set in the bitset
 * @param left the bitset in question
 * @param a pointer to a u32 where the number of elements in the indice set
 * will be stored
 * @return a vector of indices
 */
u32 *bitset_indices(bitset_t *left, u32 *vertex_count);

/**
 * Compute the sum of the parts of the bitset. The actual value of the sum does not mean
 * anything special and you should not rely on the value, except for the following values:
 *  - A 0 value indicates that all bits are 0
 *  - A non-zero value indicates that not all bits are 0
 * @param b the bitset
 * @return the sum of parts
 */
u32 bitset_sum(bitset_t *b);

/**
 * Set all the bits to the value
 * @param b the bitset
 * @param v the value
 */
void bitset_all(bitset_t *b, bool v);

/**
 * Return if an bit is up
 * @param b the bitset
 * @return
 */
bool bitset_any(bitset_t *b);

#define COPNV2_BITFIELD_H

#endif //COPNV2_BITFIELD_H
