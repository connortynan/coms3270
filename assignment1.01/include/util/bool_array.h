#pragma once

#include <stdlib.h>
#include <stdint.h>

#define PACKED_BOOL_CHUNK_BITS 16
#define EXPAND_UINT_TYPE(bits) uint##bits##_t
#define PACKED_BOOL_UINT_TYPE(bits) EXPAND_UINT_TYPE(bits)

_Static_assert(
    PACKED_BOOL_CHUNK_BITS == 8 || PACKED_BOOL_CHUNK_BITS == 16 ||
        PACKED_BOOL_CHUNK_BITS == 32 || PACKED_BOOL_CHUNK_BITS == 64,
    "PACKED_BOOL_CHUNK_BITS must be one of 8, 16, 32, or 64 for a valid stdint.h type.");

typedef PACKED_BOOL_UINT_TYPE(PACKED_BOOL_CHUNK_BITS) packed_bool_chunk_t;

/**
 * @struct packed_bool_array
 *
 * @brief array of boolean values (0/1) packed into chunks of `PACKED_BOOL_CHUNK_BITS` bits for memory efficiency
 *
 * @var _data pointer to malloced array (internal use only)
 * @var _chunk_n number of chunks in array, used to keep track of memory allocation (internal use only)
 */
typedef struct
{
    packed_bool_chunk_t *_data; /**< INTERNAL USE ONLY */
    size_t _chunk_n;            /**< INTERNAL USE ONLY */
} packed_bool_array;

/**
 * @brief Reserve memory to fit `n` booleans in the given array pointer
 *
 * @param arr bool array
 * @param n number of boolean values to reserve space for (in bits)
 *
 * @return success code
 *  0 = success
 *  1 = memory allocation error
 */
int bool_array_reserve(packed_bool_array *arr, size_t n)
{
    size_t chunks = (n + PACKED_BOOL_CHUNK_BITS - 1) / PACKED_BOOL_CHUNK_BITS;
    arr->_data = (packed_bool_chunk_t *)malloc(chunks * sizeof(*arr));

    arr->_chunk_n = chunks;

    if (arr->_data)
        return 1;

    return 0;
}

/**
 * @brief sets all elements of the boolean array to the given value (0/1)
 *
 * @param arr bool array
 * @param val boolean value (0/1) that will be set for all elements in the array
 * @return success code
 *  0 = success
 *  1 = error (unused)aaZAA
 *
 * @note for current implementation: modifies extra bits at the end of final chunk, but those shouldn't be accessed anyway under normal use
 */
int bool_array_fill(packed_bool_array *arr, int val)
{
    size_t i;
    packed_bool_chunk_t fill_chunk = (val ? ~0 : 0);

    for (i = 0; i < arr->_chunk_n; i++)
    {
        arr->_data[i] = fill_chunk;
    }

    return 0;
}

/**
 * @brief Set the given bit of the array to a boolean (0/1) value
 *
 * @param arr bool array
 * @param index index of boolean value to set (in bits)
 * @param val boolean value (0/1) to set in given index (all non-zero values will be set to 1 by default)
 *
 * @return success code
 *  0 = success
 *  1 = error (unused)
 */
int bool_array_set(packed_bool_array *arr, size_t index, int val)
{
    val = val ? 1 : 0;

    packed_bool_chunk_t *chunk = &arr->_data[index / PACKED_BOOL_CHUNK_BITS];
    uint8_t bit = index % PACKED_BOOL_CHUNK_BITS;

    *chunk &= ~(1 << bit);
    *chunk |= (val << bit);

    return 0;
}

/**
 * @brief retrieve the boolean value at the given index
 *
 * @param arr bool array
 * @param index the index of the desired boolean bit
 *
 * @return boolean value at given index
 */
int bool_array_get(packed_bool_array *arr, size_t index)
{
    return ((arr->_data[index / PACKED_BOOL_CHUNK_BITS]) >> (index % PACKED_BOOL_CHUNK_BITS)) & 1;
}