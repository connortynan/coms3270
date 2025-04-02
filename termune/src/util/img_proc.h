#pragma once

#include <algorithm>
#include "util/grid.h"
#include "util/generic_utils.h"

template <typename T>
Grid<T> gaussian_blur(Grid<T> &img, bool inplace = false)
{
    static const Grid<int> gaussian_kernel(3, 3, {1, 2, 1, 2, 4, 2, 1, 2, 1});
    static constexpr int gaussian_kernel_sum = 16;

    std::size_t width = img.width();
    std::size_t height = img.height();

    Grid<T> output(width, height, 0);

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            int sum = 0;

            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    std::size_t nx = std::clamp<int>(x + dx, 0, static_cast<int>(width) - 1);
                    std::size_t ny = std::clamp<int>(y + dy, 0, static_cast<int>(height) - 1);

                    int kernel_value = gaussian_kernel(dx + 1, dy + 1);
                    sum += img(nx, ny) * kernel_value;
                }
            }

            output(x, y) = static_cast<T>(sum / gaussian_kernel_sum);
        }
    }

    if (inplace)
    {
        img = std::move(output);
        return img;
    }
    else
    {
        return output;
    }
}
