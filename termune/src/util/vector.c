#include "vector.h"

/**
 * @brief Initializes a new vector with a specified initial capacity and element size.
 */
vector *vector_init(size_t initial_capacity, size_t element_size)
{
    vector *v = (vector *)malloc(sizeof(vector));
    if (!v)
        return NULL;

    v->_capacity = initial_capacity > 0 ? initial_capacity : VECTOR_DEFAULT_CAPACITY;
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
 */
int vector_destroy(vector *v)
{
#ifdef VECTOR_EXTRA_CHECKS
    if (!v)
        return 1;
#endif // VECTOR_EXTRA_CHECKS

    free(v->_elems);
    free(v);
    return 0;
}

/**
 * @brief Reserves additional capacity for the vector.
 */
int vector_reserve(vector *v, size_t new_capacity)
{
#ifdef VECTOR_EXTRA_CHECKS
    if (!v || new_capacity <= v->size)
        return 1;
#endif // VECTOR_EXTRA_CHECKS

    void *new_data = realloc(v->_elems, new_capacity * v->_elem_size);
    if (!new_data)
        return 1;

    v->_elems = new_data;
    v->_capacity = new_capacity;

    return 0;
}

/**
 * @brief Returns a pointer to the element at a specific index.
 */
void *vector_at(vector *v, size_t idx)
{
#ifdef VECTOR_EXTRA_CHECKS
    if (!v || idx >= v->size)
        return NULL;
#endif // VECTOR_EXTRA_CHECKS

    return (char *)v->_elems + (idx * v->_elem_size);
}

/**
 * @brief Pushes a new element to the back of the vector.
 */
int vector_push_back(vector *v, const void *element)
{
#ifdef VECTOR_EXTRA_CHECKS
    if (!v || !element)
        return 1;
#endif // VECTOR_EXTRA_CHECKS

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
 */
void vector_pop_back(vector *v)
{
#ifdef VECTOR_EXTRA_CHECKS
    if (!v || v->size == 0)
        return;
#endif // VECTOR_EXTRA_CHECKS

    v->size--;
}
