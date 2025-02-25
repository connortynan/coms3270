#pragma once

#include <stdlib.h>
#include <string.h>

/// Clamps a value `x` between `min` and `max`.
#define VALUE_CLAMP(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))

/**
 * @brief Performs linear interpolation (lerp) between two values.
 *
 * This function interpolates between `a` and `b` using the interpolation factor `t`.
 *
 * @param t Interpolation factor (usually between 0 and 1).
 * @param a Start value.
 * @param b End value.
 * @return Interpolated value between `a` and `b`.
 */
static inline float util_lerp(float t, float a, float b)
{
    return a + (b - a) * t;
}

/**
 * @brief Computes the dot product of two 2D vectors.
 *
 * This function calculates the dot product of two 2D vectors (ax, ay) and (bx, by).
 *
 * @param ax X-component of the first vector.
 * @param ay Y-component of the first vector.
 * @param bx X-component of the second vector.
 * @param by Y-component of the second vector.
 * @return The dot product of the two vectors.
 */
static inline float util_dot(const float ax, const float ay, const float bx, const float by)
{
    return ax * bx + ay * by;
}

/**
 * @brief Swaps two elements in memory.
 *
 * This function swaps two elements of size `element_size` in memory.
 *
 * @param a Pointer to the first element.
 * @param b Pointer to the second element.
 * @param element_size Size of the elements in bytes.
 */
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