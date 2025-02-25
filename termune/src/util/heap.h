#pragma once

#include <stddef.h>

/**
 * @brief Comparison function for heap elements.
 *
 * Should return a negative value if a < b, 0 if a == b, and a positive value if a > b.
 *
 * @var heap_comp_func
 */
typedef int (*heap_comp_func)(const void *a, const void *b, void *context);

/**
 * @brief Opaque structure for the heap
 */
typedef struct heap heap;

/**
 * @brief Initializes a new heap.
 *
 * @param initial_capacity number of elements to initialize the heap to store, will use default value for any non-positive input
 * @param element_size Size of each element in bytes
 * @param comp_func Function pointer to compare elements in the heap to extract the minimum
 * @param context User-defined context for comparison function
 * @return Pointer to the created heap or NULL on failure
 */
heap *heap_init(size_t initial_capacity, size_t element_size, heap_comp_func comp_func, void *context);

/**
 * @brief Destroys the heap and frees associated memory.
 *
 * @param h Pointer to the heap
 * @return 0 on success, or 1 if the heap is NULL
 */
int heap_destroy(heap *h);

/**
 * @brief Inserts a new element into the heap.
 *
 * @param h Pointer to the heap
 * @param element Pointer to the element to be inserted
 * @return 0 on success, or 1 if memory allocation fails
 */
int heap_insert(heap *h, const void *element);

/**
 * @brief Peeks at the minimum element in the heap without removing it.
 *
 * @param h Pointer to the heap
 * @return Pointer to the minimum element, or NULL if the heap is empty
 */
void *heap_peek(const heap *h);

/**
 * @brief Extracts and removes the minimum element from the heap.
 *
 * @param h Pointer to the heap
 * @param result Pointer that the minimum element will be written to.
 * @return 0 if successfully polled, or 1 on error or if heap is empty
 */
int heap_poll(heap *h, void *result);

/**
 * @brief Returns the number of items in the heap
 *
 * @param h Pointer to the heap
 * @return NUmber of elements in the heap
 */
size_t heap_size(heap *h);
