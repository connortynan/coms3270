#pragma once

#include <stdlib.h>
#include <string.h>

/// Default initial capacity of the vector
#define VECTOR_DEFAULT_CAPACITY 16

/// Perform extra error checking on vectors (slower but useful to debug)
#define VECTOR_EXTRA_CHECKS 0

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
vector *vector_init(size_t initial_capacity, size_t element_size);

/**
 * @brief Destroys the vector and frees associated memory.
 *
 * @param v Pointer to the vector
 * @return 0 on success, or 1 if the vector is NULL
 */
int vector_destroy(vector *v);

/**
 * @brief Reserves additional capacity for the vector.
 *
 * @param v Pointer to the vector
 * @param new_capacity The new capacity to reserve
 * @return 0 on success, or 1 if memory allocation fails or new_capacity is invalid
 */
int vector_reserve(vector *v, size_t new_capacity);

/**
 * @brief Returns a pointer to the element at a specific index.
 *
 * @param v Pointer to the vector
 * @param idx Index of the element
 * @return Pointer to the element, or NULL if the index is out of bounds
 */
void *vector_at(vector *v, size_t idx);

/**
 * @brief Pushes a new element to the back of the vector.
 *
 * @param v Pointer to the vector
 * @param element Pointer to the element to push
 * @return 0 on success, or 1 if memory allocation fails
 */
int vector_push_back(vector *v, const void *element);

/**
 * @brief Removes the last element from the vector.
 *
 * @param v Pointer to the vector
 */
void vector_pop_back(vector *v);
