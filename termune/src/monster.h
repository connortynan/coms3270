#pragma once

#include <stdint.h>

#include "common.h"
#include "dungeon.h"

typedef uint32_t monster_distance_map[DUNGEON_WIDTH][DUNGEON_HEIGHT];

/**
 * @brief Generates the non-tunneling distance map for monsters.
 *
 * This function computes distances for non-tunneling monsters, considering only
 * traversable terrain when calculating paths.
 *
 * @param dist_map Pointer to the monster distance map to populate.
 * @param dungeon Pointer to the dungeon data used for distance calculations.
 * @return Returns 0 on success, or a non-zero value if an error occurs.
 */
int monster_generate_nontunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon);

/**
 * @brief Generates the tunneling distance map for monsters.
 *
 * This function computes distances for tunneling monsters, allowing movement
 * through all terrain types, including walls.
 *
 * @param dist_map Pointer to the monster distance map to populate.
 * @param dungeon Pointer to the dungeon data used for distance calculations.
 * @return Returns 0 on success, or a non-zero value if an error occurs.
 */
int monster_generate_tunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon);

/**
 * @brief Prints the monster distance map to the console.
 *
 * This function outputs the distance map, typically for debugging purposes.
 *
 * @param dist_map Pointer to the monster distance map to print.
 * @param dungeon Pointer to the dungeon data used for distance calculations.
 * @return Returns 0 on success, or a non-zero value if an error occurs.
 */
int monster_print_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon);
