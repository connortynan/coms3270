#pragma once

#include <stdlib.h>
#include <string.h>

#define VALUE_CLAMP(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))

static inline float util_lerp(float t, float a, float b)
{
    return a + (b - a) * t;
}

static inline float util_dot(const float ax, const float ay, const float bx, const float by)
{
    return ax * bx + ay * by;
}

static inline void util_swap_elements(void *a, void *b, size_t element_size)
{
    void *temp = malloc(element_size);
    if (temp == NULL)
        return;
    memcpy(temp, a, element_size);
    memcpy(a, b, element_size);
    memcpy(b, temp, element_size);
    free(temp);
}

/**
 * @brief Shuffles an array in-place using the Fisher-Yates shuffle algorithm.
 *
 * This function randomizes the order of the elements in the provided array.
 *
 * @param arr Pointer to the array to be shuffled.
 * @param n The number of elements in the array.
 * @param size The size of each element in the array, in bytes.
 */
static inline void util_shuffle(void *arr, size_t n, size_t size)
{
    if (arr == NULL || n < 2 || size == 0)
    {
        return; // Nothing to shuffle
    }

    char *array = (char *)arr;

    for (size_t i = n - 1; i > 0; i--)
    {
        size_t j = rand() % (i + 1);

        char *a = array + i * size;
        char *b = array + j * size;

        char temp[size];
        memcpy(temp, a, size);
        memcpy(a, b, size);
        memcpy(b, temp, size);
    }
}