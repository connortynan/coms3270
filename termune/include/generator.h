#pragma once

#include <stdint.h>

#include "common.h"
#include "dungeon.h"

/**
 * Parameters to generate a dungeon to make dungeon generation paramters modifiable easier
 */
typedef struct generator_parameters
{
    uint16_t min_room_width;
    uint16_t max_room_width;

    uint16_t min_room_height;
    uint16_t max_room_height;

    uint16_t min_num_rooms;
    uint16_t max_num_rooms;

    uint16_t min_num_stairs;
    uint16_t max_num_stairs;

    uint8_t min_rock_hardness;
    uint8_t max_rock_hardness;

    uint8_t rock_hardness_smoothness;
    float rock_hardness_noise_amount;
} generator_parameters;

/**
 * @brief Initiliaze a dungeon given the parameters, with a random number of rooms in the range uniformly spread out across the dungeon.
 *
 * The algorithm works as follows:
 *  - Generate a "bucket" of n possible rooms defined by (center_x, center_y, width, height),
 *    where width and height are in the bound set by the parameters, and x and y are randomly chosen within the dungeon size
 *  - Select the number of rooms to place from the range in the parameter (denoted as `r`)
 *  - Repeat until we have placed `r` rooms:
 *      - Go down the list of rooms in the bucket and see if we can place it
 *      - If yes, place it and keep track of the rooms that have been placed, and go to next room in bucket
 *      - If no, skip to next one
 *      - If reach end of bucket:
 *          - Undo last placed room and retreat to the rooms after in the bucket and repeat
 *          - If no rooms have been placed and all dont work:
 *              - Bucket is impossible, restart algortihm with new generated bucket
 *  - Assign each room a hardness
 *  - Set all rock based on nearest rooms hardness
 *  - Blur rock hardness
 *  - Add subtle perlin noise to all rocks hardness
 *  - Order rooms in a way that reduces distance between rooms, while accounting for a loop from last room back to first
 *  - Generate path between room centers, overriding rocks with corrdor floor
 *  - Add stairs randomly on corridor cells, prioritizing distance between them
 *
 * @param dungeon A pointer to the generated dungeon
 * @param params A pointer to the `generator_parameters` struct defining the rules of generation
 *
 * @return success code
 * 0 = successfully generated code
 * 1 = failed dungeon generation
 */
int generator_generate_dungeon(dungeon_data *dungeon, const generator_parameters *params);