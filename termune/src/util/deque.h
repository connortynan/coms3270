#pragma once

#include <stddef.h>

/// Perform extra error checking on deque (slower but useful for debugging)
#define DEQUE_EXTRA_CHECKS 0

/**
 * @brief Opaque structure for the deque
 */
typedef struct deque deque;

/**
 * @brief Initializes a new deque with a specified element size.
 *
 * @param element_size Size of each element in bytes
 * @return Pointer to the created deque or NULL on failure
 */
deque *deque_init(size_t element_size);

/**
 * @brief Destroys the deque and frees associated memory.
 *
 * @param d Pointer to the deque
 * @return 0 on success, or 1 if the deque is NULL
 */
int deque_destroy(deque *d);

/**
 * @brief Pushes a new element to the front of the deque.
 *
 * @param d Pointer to the deque
 * @param element Pointer to the element to push
 * @return 0 on success, or 1 if memory allocation fails
 */
int deque_push_front(deque *d, const void *element);

/**
 * @brief Pushes a new element to the back of the deque.
 *
 * @param d Pointer to the deque
 * @param element Pointer to the element to push
 * @return 0 on success, or 1 if memory allocation fails
 */
int deque_push_back(deque *d, const void *element);

/**
 * @brief Removes the front element from the deque.
 *
 * @param d Pointer to the deque
 */
void deque_pop_front(deque *d);

/**
 * @brief Removes the last element from the deque.
 *
 * @param d Pointer to the deque
 */
void deque_pop_back(deque *d);

/**
 * @brief Returns a pointer to the front element of the deque.
 *
 * @param d Pointer to the deque
 * @return Pointer to the front element, or NULL if empty
 */
void *deque_front(deque *d);

/**
 * @brief Returns a pointer to the last element of the deque.
 *
 * @param d Pointer to the deque
 * @return Pointer to the last element, or NULL if empty
 */
void *deque_back(deque *d);
