#pragma once

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Applies a Gaussian blur to a grayscale image.
 *
 * This function blurs the input image using a 3x3 Gaussian kernel. The operation is applied to each pixel of
 * the image, where the new pixel value is computed as a weighted sum of its neighbors.
 * The result is stored back in the input image.
 *
 * @param img Pointer to the input image data (grayscale).
 * @param w Width of the image.
 * @param h Height of the image.
 *
 * @return 0 on success, 1 on failure (memory allocation error).
 *
 * @note The image is assumed to be in a single channel (grayscale), with pixel values in the range [0, 255].
 */
int gaussian_blur(uint8_t *img, size_t w, size_t h);
