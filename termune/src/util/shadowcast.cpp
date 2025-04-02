#include "util/shadowcast.h"

#include <cmath>
#include <cstdio>
#include <algorithm>

namespace ShadowCast
{
    static constexpr float EPSILON = 0.0001f;

    // Octant transformation matrix
    static const int MULT[4][8] = {
        {1, 0, 0, -1, -1, 0, 0, 1}, // xx
        {0, 1, -1, 0, 0, -1, 1, 0}, // xy
        {0, 1, 1, 0, 0, -1, -1, 0}, // yx
        {1, 0, 0, 1, -1, 0, 0, -1}  // yy
    };

    // Checks if a cell is outside the map or blocks light
    inline bool is_blocked(const Grid<unsigned char> &solid_map, int x, int y)
    {
        return static_cast<std::size_t>(x) >= solid_map.width() ||
               static_cast<std::size_t>(y) >= solid_map.height() ||
               solid_map(x, y);
    }

    // Marks a cell as visible in the lightmap
    inline void set_visible(Lightmap &map, int x, int y)
    {
        if (static_cast<std::size_t>(x) < map.width() &&
            static_cast<std::size_t>(y) < map.height())
        {
            map(x, y) = true;
        }
    }

    // Recursive shadowcasting function
    static void cast_light(
        const Grid<unsigned char> &solid_map,
        Lightmap &visible,
        int origin_x, int origin_y,
        int row,
        float start_slope, float end_slope,
        std::size_t radius,
        int xx, int xy, int yx, int yy)
    {
        if (start_slope < end_slope)
            return;

        int radius_sq = static_cast<int>(radius * radius);

        for (int j = row; static_cast<std::size_t>(j) <= radius; ++j)
        {
            int dx = -j - 1;
            int dy = -j;
            bool blocked = false;
            float new_start_slope = start_slope;

            while (dx <= 0)
            {
                ++dx;

                int X = origin_x + dx * xx + dy * xy;
                int Y = origin_y + dx * yx + dy * yy;

                float l_slope = (dx - 0.5f) / (dy + 0.5f);
                float r_slope = (dx + 0.5f) / (dy - 0.5f);

                if (start_slope + EPSILON < r_slope)
                    continue;
                if (end_slope - EPSILON > l_slope)
                    break;

                int dist_sq = dx * dx + dy * dy;
                if (dist_sq < radius_sq)
                {
                    if (!is_blocked(solid_map, X, Y))
                    {
                        set_visible(visible, X, Y);
                    }
                }

                if (blocked)
                {
                    if (is_blocked(solid_map, X, Y))
                    {
                        new_start_slope = r_slope;
                        continue;
                    }
                    else
                    {
                        blocked = false;
                        start_slope = new_start_slope;
                    }
                }
                else if (is_blocked(solid_map, X, Y))
                {
                    blocked = true;
                    cast_light(solid_map, visible, origin_x, origin_y,
                               j + 1, start_slope, l_slope, radius,
                               xx, xy, yx, yy);
                    new_start_slope = r_slope;
                }
            }

            if (blocked)
                break;
        }
    }

    void update_lightmap(
        const Grid<unsigned char> &solid_map,
        Lightmap &visible,
        std::size_t origin_x,
        std::size_t origin_y,
        std::size_t radius = 0)
    {
        visible.fill(false); // Clear previous visibility

        set_visible(visible, static_cast<int>(origin_x), static_cast<int>(origin_y)); // Light source is always visible

        for (int oct = 0; oct < 8; ++oct)
        {
            cast_light(solid_map, visible,
                       static_cast<int>(origin_x), static_cast<int>(origin_y),
                       1, 1.0f, 0.0f, radius,
                       MULT[0][oct], MULT[1][oct],
                       MULT[2][oct], MULT[3][oct]);
        }
    }

    Lightmap solve_lightmap(
        const Grid<unsigned char> &solid_map,
        std::size_t origin_x,
        std::size_t origin_y,
        std::size_t radius = 0)
    {
        Lightmap result(solid_map.width(), solid_map.height(), false);
        update_lightmap(solid_map, result, origin_x, origin_y, radius);
        return result;
    }
} // namespace ShadowCast
