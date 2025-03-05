#include "util/shadowcast.h"
#include "util/deque.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define FOV_RADIUS DUNGEON_WIDTH
#define EPSILON 0.0001f // More precise tolerance for floating-point comparisons

// Octant transformations
static const int MULT[4][8] = {
    {1, 0, 0, -1, -1, 0, 0, 1}, // xx
    {0, 1, -1, 0, 0, -1, 1, 0}, // xy
    {0, 1, 1, 0, 0, -1, -1, 0}, // yx
    {1, 0, 0, 1, -1, 0, 0, -1}  // yy
};

// Check if a tile is a wall (blocked)
static int is_blocked(dungeon_data *dungeon, int x, int y)
{
    return (x < 0 || y < 0 || x >= DUNGEON_WIDTH || y >= DUNGEON_HEIGHT ||
            dungeon->cell_types[x][y] == CELL_ROCK);
}

// Mark a tile as visible (only for open space)
static void set_lit(lightmap *map, int x, int y)
{
    if (x >= 0 && y >= 0 && x < DUNGEON_WIDTH && y < DUNGEON_HEIGHT)
    {
        (*map)[x][y] = 1;
    }
}

// Recursive shadowcasting function with **strict** wall handling
static void cast_light(lightmap *map, dungeon_data *dungeon, int cx, int cy, int row,
                       float start_slope, float end_slope, int radius,
                       int xx, int xy, int yx, int yy)
{
    if (start_slope < end_slope)
        return;

    int radius_squared = radius * radius;

    for (int j = row; j <= radius; j++)
    {
        int dx = -j - 1;
        int dy = -j;
        int blocked = 0;
        float new_start_slope = start_slope;

        while (dx <= 0)
        {
            dx++;
            int X = cx + dx * xx + dy * xy;
            int Y = cy + dx * yx + dy * yy;

            float l_slope = (dx - 0.5f) / (dy + 0.5f);
            float r_slope = (dx + 0.5f) / (dy - 0.5f);

            if (start_slope + EPSILON < r_slope)
                continue;
            if (end_slope - EPSILON > l_slope)
                break;

            if ((dx * dx + dy * dy) < radius_squared)
            {
                if (!is_blocked(dungeon, X, Y))
                {
                    set_lit(map, X, Y); // **Only mark open space as visible**
                }
            }

            if (blocked)
            {
                if (is_blocked(dungeon, X, Y))
                {
                    new_start_slope = r_slope;
                    continue;
                }
                else
                {
                    blocked = 0;
                    start_slope = new_start_slope;
                }
            }
            else if (is_blocked(dungeon, X, Y))
            {
                blocked = 1;
                cast_light(map, dungeon, cx, cy, j + 1, start_slope, l_slope, radius, xx, xy, yx, yy);
                new_start_slope = r_slope;
            }
        }

        if (blocked)
            break;
    }
}

// Main function to compute FOV using shadowcasting
int shadowcast_solve_lightmap(lightmap *map, dungeon_data *dungeon)
{
    int cx = dungeon->pc_x;
    int cy = dungeon->pc_y;

    // Reset visibility map
    for (int i = 0; i < DUNGEON_WIDTH; i++)
    {
        for (int j = 0; j < DUNGEON_HEIGHT; j++)
        {
            (*map)[i][j] = 0; // Mark all as unseen
        }
    }

    // Player's position is always visible
    set_lit(map, cx, cy);

    // Process each octant
    for (int i = 0; i < 8; i++)
    {
        cast_light(map, dungeon, cx, cy, 1, 1.0f, 0.0f, FOV_RADIUS,
                   MULT[0][i], MULT[1][i], MULT[2][i], MULT[3][i]);
    }

    return 0; // Success
}

// Display function for debugging
void shadowcast_display_lightmap(lightmap *map)
{
    for (int y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (int x = 0; x < DUNGEON_WIDTH; x++)
        {
            printf("%c", (*map)[x][y] ? '.' : '#'); // Open space is visible, walls stay hidden
        }
        printf("\n");
    }
}
