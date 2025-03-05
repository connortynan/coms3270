#include "deque.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Node structure for the linked list deque
 */
typedef struct deque_node
{
    void *data;
    struct deque_node *next;
    struct deque_node *prev;
} deque_node;

/**
 * @brief Deque data structure definition
 */
struct deque
{
    deque_node *head;
    deque_node *tail;
    size_t _elem_size;
    size_t size;
};

deque *deque_init(size_t element_size)
{
    deque *d = (deque *)malloc(sizeof(deque));
    if (!d)
        return NULL;

    d->head = d->tail = NULL;
    d->_elem_size = element_size;
    d->size = 0;

    return d;
}

int deque_destroy(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d)
        return 1;
#endif

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

int deque_push_front(deque *d, const void *element)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || !element)
        return 1;
#endif

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

int deque_push_back(deque *d, const void *element)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || !element)
        return 1;
#endif

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

void deque_pop_front(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return;
#endif

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

void deque_pop_back(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return;
#endif

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

void *deque_front(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return NULL;
#endif

    return d->head ? d->head->data : NULL;
}

void *deque_back(deque *d)
{
#ifdef DEQUE_EXTRA_CHECKS
    if (!d || d->size == 0)
        return NULL;
#endif

    return d->tail ? d->tail->data : NULL;
}

int deque_empty(deque *d)
{
    return !d || d->size == 0;
}