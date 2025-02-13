#pragma once

#include <stdlib.h>
#include <string.h>

#include "util/util.h"
#include "util/vector.h"

/**
 * @brief Comparison function for heap elements.
 *
 * Should return a negative value if a < b, 0 if a == b, and a positive value if a > b.
 *
 * @var heap_comp_func
 */
typedef int (*heap_comp_func)(const void *a, const void *b);

/**
 * @brief Heap data structure which works for generic types using type agnostic vectors as a backend
 */
typedef struct heap
{
    vector *_vec;            /**< Vector to store heap elements */
    heap_comp_func _compare; /**< Function to compare elements */
} heap;

static inline void _heapify_up(heap *h, size_t idx)
{
    while (idx > 0)
    {
        size_t parent_index = (idx - 1) / 2;

        void *current_elem = vector_at(h->_vec, idx);
        void *parent_elem = vector_at(h->_vec, parent_index);

        if (h->_compare(current_elem, parent_elem) >= 0)
            break;

        util_swap_elements(current_elem, parent_elem, h->_vec->_elem_size);
        idx = parent_index;
    }
}

static inline void _heapify_down(heap *h, size_t idx)
{
    while (idx < h->_vec->size)
    {
        size_t left_child = 2 * idx + 1;
        size_t right_child = 2 * idx + 2;
        size_t smallest = idx;

        void *current_elem = vector_at(h->_vec, idx);

        if (left_child < h->_vec->size)
        {
            void *left_elem = vector_at(h->_vec, left_child);
            if (h->_compare(left_elem, current_elem) < 0)
            {
                smallest = left_child;
            }
        }

        if (right_child < h->_vec->size)
        {
            void *right_elem = vector_at(h->_vec, right_child);
            if (h->_compare(right_elem, vector_at(h->_vec, smallest)) < 0)
            {
                smallest = right_child;
            }
        }

        if (smallest == idx)
            break;

        util_swap_elements(current_elem, vector_at(h->_vec, smallest), h->_vec->_elem_size);
        idx = smallest;
    }
}

/**
 * @brief Initializes a new heap.
 *
 * @param initial_capacity number of elements to initilize the heap to store, will use default value for any non-positive input
 * @param element_size Size of each element in bytes
 * @param comp_func Functoin pointer to compare elements in the heap to extract the minimum
 * @return Pointer to the created heap or NULL on failure
 */
static inline heap *heap_init(size_t initial_capacity, size_t element_size, heap_comp_func comp_func)
{
    if (element_size == 0 || !comp_func)
        return NULL;

    heap *h = (heap *)malloc(sizeof(heap));
    if (!h)
        return NULL;

    h->_vec = vector_init(initial_capacity, element_size);
    if (!h->_vec)
    {
        free(h);
        return NULL;
    }

    h->_compare = comp_func;
    return h;
}

/**
 * @brief Destroys the heap and frees associated memory.
 *
 * @param h Pointer to the heap
 * @return 0 on success, or 1 if the heap is NULL
 */
static inline int heap_destroy(heap *h)
{
    if (!h)
        return 1;

    vector_destroy(h->_vec);
    free(h);
    return 0;
}

/**
 * @brief Inserts a new element into the heap.
 *
 * @param h Pointer to the heap
 * @param element Pointer to the element to be inserted
 * @return 0 on success, or 1 if memory allocation fails
 */
static inline int heap_insert(heap *h, const void *element)
{
    if (!h || !element)
        return 1;

    if (vector_push_back(h->_vec, element))
        return 1;

    _heapify_up(h, h->_vec->size - 1);
    return 0;
}

/**
 * @brief Peeks at the minimum element in the heap without removing it.
 *
 * @param h Pointer to the heap
 * @return Pointer to the minimum element, or NULL if the heap is empty
 */
static inline void *heap_peek(const heap *h)
{
    if (!h || h->_vec->size == 0)
        return NULL;

    return vector_at(h->_vec, 0);
}

/**
 * @brief Extracts and removes the minimum element from the heap.
 *
 * @param h Pointer to the heap
 * @param result Pointer that the minimum element will be written to.
 * @return 0 if successfully polled, or 1 on error or if heap is empty
 */
static inline int heap_poll(heap *h, void *result)
{
    if (!h || h->_vec->size == 0)
        return 1;

    memcpy(result, vector_at(h->_vec, 0), h->_vec->_elem_size);

    // Move the last element to the root and heapify down
    void *last_elem = vector_at(h->_vec, h->_vec->size - 1);
    memcpy(vector_at(h->_vec, 0), last_elem, h->_vec->_elem_size);
    h->_vec->size--;

    _heapify_down(h, 0);
    return 0;
}