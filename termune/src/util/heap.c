#include "heap.h"
#include <stdlib.h>
#include <string.h>
#include "util/generic_utils.h"
#include "util/vector.h"

/**
 * @brief Heap data structure which works for generic types using type agnostic vectors as a backend
 */
struct heap
{
    vector *_vec;            /**< Vector to store heap elements */
    heap_comp_func _compare; /**< Function to compare elements */
    void *context;           /**< Context for comparison function */
};

static inline void _heapify_up(heap *h, size_t idx)
{
    while (idx > 0)
    {
        size_t parent_index = (idx - 1) / 2;
        void *current_elem = vector_at(h->_vec, idx);
        void *parent_elem = vector_at(h->_vec, parent_index);

        if (h->_compare(current_elem, parent_elem, h->context) >= 0)
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

        if (left_child < h->_vec->size && h->_compare(vector_at(h->_vec, left_child), current_elem, h->context) < 0)
            smallest = left_child;

        if (right_child < h->_vec->size && h->_compare(vector_at(h->_vec, right_child), vector_at(h->_vec, smallest), h->context) < 0)
            smallest = right_child;

        if (smallest == idx)
            break;

        util_swap_elements(current_elem, vector_at(h->_vec, smallest), h->_vec->_elem_size);
        idx = smallest;
    }
}

heap *heap_init(size_t initial_capacity, size_t element_size, heap_comp_func comp_func, void *context)
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
    h->context = context;
    return h;
}

int heap_destroy(heap *h)
{
    if (!h)
        return 1;

    vector_destroy(h->_vec);
    free(h);
    return 0;
}

int heap_insert(heap *h, const void *element)
{
    if (!h || !element)
        return 1;

    if (vector_push_back(h->_vec, element))
        return 1;

    _heapify_up(h, h->_vec->size - 1);
    return 0;
}

void *heap_peek(const heap *h)
{
    if (!h || h->_vec->size == 0)
        return NULL;

    return vector_at(h->_vec, 0);
}

int heap_poll(heap *h, void *result)
{
    if (!h || h->_vec->size == 0 || !result)
        return 1;

    memcpy(result, vector_at(h->_vec, 0), h->_vec->_elem_size);
    memcpy(vector_at(h->_vec, 0), vector_at(h->_vec, h->_vec->size - 1), h->_vec->_elem_size);
    h->_vec->size--;

    _heapify_down(h, 0);
    return 0;
}

size_t heap_size(heap *h)
{
    return h->_vec->size;
}