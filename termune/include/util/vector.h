#pragma once

#include <stdlib.h>
#include <string.h>

/// Default initial capacity of the vector
#define VECTOR_DEFAULT_CAPACTIY 16

/**
 * @brief Vector data structure definition
 */
typedef struct vector
{
    void *_elems;      /**< Pointer to the vector elements */
    size_t _capacity;  /**< Current capacity of the vector */
    size_t _elem_size; /**< Size of each element in bytes */
    size_t size;       /**< Current number of elements in the vector */
} vector;

/**
 * @brief Initializes a new vector with a specified initial capacity and element size.
 *
 * @param initial_capacity Initial capacity of the vector, or default if non-positive
 * @param element_size Size of each element in bytes
 * @return Pointer to the created vector or NULL on failure
 */
static inline vector *vector_init(size_t initial_capacity, size_t element_size)
{
    vector *v = (vector *)malloc(sizeof(vector));
    if (!v)
        return NULL;

    v->_capacity = initial_capacity > 0 ? initial_capacity : VECTOR_DEFAULT_CAPACTIY;
    v->_elem_size = element_size;

    v->_elems = malloc(v->_capacity * v->_elem_size);
    if (!v->_elems)
    {
        free(v);
        return NULL;
    }

    v->size = 0;
    return v;
}

/**
 * @brief Destroys the vector and frees associated memory.
 *
 * @param v Pointer to the vector
 * @return 0 on success, or 1 if the vector is NULL
 */
static inline int vector_destroy(vector *v)
{
    if (!v)
        return 1;

    free(v->_elems);
    free(v);
    return 0;
}

/**
 * @brief Reserves additional capacity for the vector.
 *
 * @param v Pointer to the vector
 * @param new_capacity The new capacity to reserve
 * @return 0 on success, or 1 if memory allocation fails or new_capacity is invalid
 */
static inline int vector_reserve(vector *v, size_t new_capacity)
{
    if (!v || new_capacity <= v->size)
        return 1;

    void *new_data = realloc(v->_elems, new_capacity * v->_elem_size);
    if (!new_data)
        return 1;

    v->_elems = new_data;
    v->_capacity = new_capacity;

    return 0;
}

/**
 * @brief Returns a pointer to the element at a specific index.
 *
 * @param v Pointer to the vector
 * @param idx Index of the element
 * @return Pointer to the element, or NULL if the index is out of bounds
 */
static inline void *vector_at(vector *v, size_t idx)
{
    if (!v || idx >= v->size)
        return NULL;

    return (char *)v->_elems + (idx * v->_elem_size);
}

/**
 * @brief Pushes a new element to the back of the vector.
 *
 * @param v Pointer to the vector
 * @param element Pointer to the element to push
 * @return 0 on success, or 1 if memory allocation fails
 */
static inline int vector_push_back(vector *v, const void *element)
{
    if (!v || !element)
        return 1;

    // Ensure enough capacity
    if (v->size == v->_capacity)
    {
        if (vector_reserve(v, v->_capacity + v->_capacity / 2))
            return 1;
    }

    void *destination = (char *)v->_elems + (v->size * v->_elem_size);
    memcpy(destination, element, v->_elem_size);
    v->size++;

    return 0;
}

/**
 * @brief Removes the last element from the vector.
 *
 * @param v Pointer to the vector
 */
static inline void vector_pop_back(vector *v)
{
    if (!v || v->size == 0)
        return;

    v->size--;
}
