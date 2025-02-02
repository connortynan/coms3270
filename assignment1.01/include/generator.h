#pragma once

#include "common.h"

#include "dungeon.h"

/**
 * All possible room types that can be generated
 *
 * Multiple shapes can be ORed together to select multiple shapes for parameters
 */
typedef enum
{
    SHAPE_RECT = 1,
    SHAPE_ROUNDED_RECT = 2,
    SHAPE_ELLIPSE = 4,
} generator_shape;

/**
 * Room data used for generating a room
 *
 * Shape must be an explicit value (non-ORed together), and corner_radius will be unused for all shapes except `ROOM_ROUNDED_RECT`
 *
 * In cases where `width` are even, `center_x` will be the cell on the left of the two cells that the true center lies on.
 * Similarly with an even `height`, `center_y` will be the top of the two cells that represent the true vertical center of the room.
 */
typedef struct
{
    generator_shape shape;
    uint16_t center_x;
    uint16_t center_y;

    uint16_t width;
    uint16_t height;

    uint16_t corner_radius;
} generator_room_data;

/**
 * Parameters to generate a dungeon to make dungeon generation paramters modifiable easier
 */
typedef struct
{
    uint16_t min_room_width;
    uint16_t max_room_width;

    uint16_t min_room_height;
    uint16_t max_room_height;

    uint16_t min_num_rooms;
    uint16_t max_num_rooms;

    uint16_t min_num_stairs;
    uint16_t max_num_stairs;

    generator_shape possible_room_shapes;
} generator_parameters;

/**
 * Initiliaze a dungeon given the parameters
 *
 * @param dungeon A pointer to the generated dungeon (expected to be preallocated)
 * @param params A pointer to the `generator_parameters` struct defining the rules of generation
 *
 * @return success code
 * 0 = successfully generated code
 * 1 = failed dungeon generation
 */
int generator_generate_dungeon(dungeon_data *dungeon, generator_parameters *params);