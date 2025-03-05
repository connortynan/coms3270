#pragma once

#include <stdint.h>
#include "common.h"
#include "dungeon.h"

/**
 * @typedef lightmap
 * @brief Represents a visibility grid, where each cell is either seen (1) or unseen (0).
 *
 * The lightmap is a 2D array of uint8_t values, corresponding to the dungeon's dimensions.
 * Values are set to `1` for visible tiles and `0` for unseen tiles.
 */
typedef uint8_t lightmap[DUNGEON_WIDTH][DUNGEON_HEIGHT];

/**
 * @brief Computes the field of view (FOV) using the recursive shadowcasting algorithm.
 *
 * This function calculates which tiles are visible from the player's position, storing
 * the results in the provided lightmap. Visibility is blocked by walls (`CELL_ROCK` tiles),
 * ensuring that light does not pass through them.
 *
 * @param map Pointer to the lightmap where visibility data will be stored.
 * @param dungeon Pointer to the dungeon structure containing map data.
 * @return 0 on success.
 *
 * @note The player's position (pc_x, pc_y) is always set as visible.
 * @details This implementation is based on the Python shadowcasting algorithm described in
 * https://www.roguebasin.com/index.php/Python_shadowcasting_implementation.
 */
int shadowcast_solve_lightmap(lightmap *map, dungeon_data *dungeon);

/**
 * @brief Displays the computed lightmap to the console for debugging.
 *
 * This function prints the visibility map, where `.` represents visible tiles
 * and `#` represents unseen tiles. It is useful for verifying the correctness of
 * the shadowcasting algorithm.
 *
 * @param map Pointer to the lightmap to be displayed.
 */
void shadowcast_display_lightmap(lightmap *map);
