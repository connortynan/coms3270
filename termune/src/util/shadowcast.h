#pragma once

#include <cstddef>
#include "util/grid.h"

namespace ShadowCast
{
    using Lightmap = Grid<bool>;

    /**
     * @brief Computes a visibility map using shadowcasting.
     *
     * @param solid_map A Grid<bool> where true means the tile blocks light.
     * @param visible The output Lightmap to be filled (in-place).
     * @param origin_x The x-coordinate of the light source.
     * @param origin_y The y-coordinate of the light source.
     * @param radius Optional maximum light radius (0 = unlimited).
     *
     * @details This implementation is based on the Python shadowcasting algorithm described in
     * https://www.roguebasin.com/index.php/Python_shadowcasting_implementation.
     */
    void update_lightmap(
        const Grid<bool> &solid_map,
        Lightmap &visible,
        std::size_t origin_x,
        std::size_t origin_y,
        std::size_t radius = 0);

    /**
     * @brief Computes and returns a visibility map using shadowcasting.
     *
     * @param solid_map A Grid<bool> where true means the tile blocks light.
     * @param origin_x The x-coordinate of the light source.
     * @param origin_y The y-coordinate of the light source.
     * @param radius Optional maximum light radius (0 = unlimited).
     * @return Lightmap A new visibility map (true = visible).
     */
    Lightmap solve_lightmap(
        const Grid<bool> &solid_map,
        std::size_t origin_x,
        std::size_t origin_y,
        std::size_t radius = 0)
    {
        Lightmap result(solid_map.width(), solid_map.height(), false);

        update_lightmap(solid_map, result, origin_x, origin_y, radius);

        return result;
    }
} // namespace ShadowCast
