#pragma once

#include <stdlib.h>
#include <string.h>

/// Perform extra error checking on deque (slower but useful for debugging)
#define DEQUE_EXTRA_CHECKS 0

/**
 * @brief Node structure for the linked list deque
 */
typedef struct deque_node
{
    void *data;              /**< Pointer to the node data */
    struct deque_node *next; /**< Pointer to the next node */
    struct deque_node *prev; /**< Pointer to the previous node */
} deque_node;

/**
 * @brief Deque data structure definition
 */
typedef struct deque
{
    deque_node *head;  /**< Pointer to the front node */
    deque_node *tail;  /**< Pointer to the back node */
    size_t _elem_size; /**< Size of each element in bytes */
    size_t size;       /**< Current number of elements in the deque */
} deque;

/**
 * @brief Initializes a new deque with a specified element size.
 *
 * @param element_size Size of each element in bytes
 * @return Pointer to the created deque or NULL on failure
 */
static inline deque *deque_init(size_t element_size)
{
    deque *d = (deque *)malloc(sizeof(deque));
    if (!d)
        return NULL;

    d->head = d->tail = NULL;
    d->_elem_size = element_size;
    d->size = 0;

    return d;
}

/**
 * @brief Destroys the deque and frees associated memory.
 *
 * @param d Pointer to the deque
 * @return 0 on success, or 1 if the deque is NULL
 */
static inline int deque_destroy(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d)
        return 1;
#endif // DEQUE_EXTRA_CHECKS

    deque_node *current = d->head;
    while (current)
    {
        deque_node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }

    free(d);
    return 0;
}

/**
 * @brief Pushes a new element to the front of the deque.
 *
 * @param d Pointer to the deque
 * @param element Pointer to the element to push
 * @return 0 on success, or 1 if memory allocation fails
 */
static inline int deque_push_front(deque *d, const void *element)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || !element)
        return 1;
#endif // DEQUE_EXTRA_CHECKS

    deque_node *new_node = (deque_node *)malloc(sizeof(deque_node));
    if (!new_node)
        return 1;

    new_node->data = malloc(d->_elem_size);
    if (!new_node->data)
    {
        free(new_node);
        return 1;
    }

    memcpy(new_node->data, element, d->_elem_size);
    new_node->next = d->head;
    new_node->prev = NULL;

    if (d->head)
        d->head->prev = new_node;
    else
        d->tail = new_node;

    d->head = new_node;
    d->size++;

    return 0;
}

/**
 * @brief Pushes a new element to the back of the deque.
 *
 * @param d Pointer to the deque
 * @param element Pointer to the element to push
 * @return 0 on success, or 1 if memory allocation fails
 */
static inline int deque_push_back(deque *d, const void *element)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || !element)
        return 1;
#endif // DEQUE_EXTRA_CHECKS

    deque_node *new_node = (deque_node *)malloc(sizeof(deque_node));
    if (!new_node)
        return 1;

    new_node->data = malloc(d->_elem_size);
    if (!new_node->data)
    {
        free(new_node);
        return 1;
    }

    memcpy(new_node->data, element, d->_elem_size);
    new_node->prev = d->tail;
    new_node->next = NULL;

    if (d->tail)
        d->tail->next = new_node;
    else
        d->head = new_node;

    d->tail = new_node;
    d->size++;

    return 0;
}

/**
 * @brief Removes the front element from the deque.
 *
 * @param d Pointer to the deque
 */
static inline void deque_pop_front(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return;
#endif // DEQUE_EXTRA_CHECKS

    if (!d->head)
        return;

    deque_node *temp = d->head;
    d->head = d->head->next;

    if (d->head)
        d->head->prev = NULL;
    else
        d->tail = NULL;

    free(temp->data);
    free(temp);
    d->size--;
}

/**
 * @brief Removes the last element from the deque.
 *
 * @param d Pointer to the deque
 */
static inline void deque_pop_back(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return;
#endif // DEQUE_EXTRA_CHECKS

    if (!d->tail)
        return;

    deque_node *temp = d->tail;
    d->tail = d->tail->prev;

    if (d->tail)
        d->tail->next = NULL;
    else
        d->head = NULL;

    free(temp->data);
    free(temp);
    d->size--;
}

/**
 * @brief Returns a pointer to the front element of the deque.
 *
 * @param d Pointer to the deque
 * @return Pointer to the front element, or NULL if empty
 */
static inline void *deque_front(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return NULL;
#endif // DEQUE_EXTRA_CHECKS

    return d->head ? d->head->data : NULL;
}

/**
 * @brief Returns a pointer to the last element of the deque.
 *
 * @param d Pointer to the deque
 * @return Pointer to the last element, or NULL if empty
 */
static inline void *deque_back(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return NULL;
#endif // DEQUE_EXTRA_CHECKS

    return d->tail ? d->tail->data : NULL;
}
