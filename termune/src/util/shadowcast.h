#pragma once

#include <cstddef>
#include "util/grid.h"

namespace ShadowCast
{
    using Lightmap = Grid<unsigned char>;

    /**
     * @details This implementation is based on the Python shadowcasting algorithm described in
     * https://www.roguebasin.com/index.php/Python_shadowcasting_implementation.
     */
    void update_lightmap(
        const Grid<unsigned char> &solid_map,
        Lightmap &visible,
        std::size_t origin_x,
        std::size_t origin_y,
        std::size_t radius);

    Lightmap solve_lightmap(
        const Grid<unsigned char> &solid_map,
        std::size_t origin_x,
        std::size_t origin_y,
        std::size_t radius);
} // namespace ShadowCast
