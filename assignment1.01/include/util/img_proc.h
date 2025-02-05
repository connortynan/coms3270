#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util/util.h"

static const int _gaussian_kernel_3[3][3] =
    {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}};
static const int _gaussian_kernel_3_sum = 16;

static const int gaussian_blur(uint8_t *img, size_t w, size_t h)
{
    uint8_t *result = (uint8_t *)malloc(w * h * sizeof(*result));
    if (!result)
        return 1;
    int16_t x, y, dx, dy, nx, ny;
    uint16_t pixel_value;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            pixel_value = 0;
            for (dx = -1; dx <= 1; dx++)
            {
                for (dy = -1; dy <= 1; dy++)
                {
                    nx = VALUE_CLAMP(x + dx, 0, w - 1);
                    ny = VALUE_CLAMP(y + dy, 0, h - 1);

                    pixel_value += img[nx + ny * w] * _gaussian_kernel_3[dx + 1][dy + 1];
                }
            }
            pixel_value /= _gaussian_kernel_3_sum;
            result[x + y * w] = pixel_value;
        }
    }

    memcpy(img, result, w * h * sizeof(*result));
    free(result);

    return 0;
}