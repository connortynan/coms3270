#pragma once

#include "common.h"

/**
 * @enum dungeon_cell_t
 *
 * @brief Enumerator of all static cell types, each with special properties
 */
typedef enum
{
    ROCK,      /**< Solid walls, cannot be traversed over */
    ROOM,      /**< Room floor */
    CORRIDOR,  /**< Corridor floor, which connects rooms */
    STAIR_UP,  /**< Upwards staircase which takes you to the dungeon connected above */
    STAIR_DOWN /**< Downwards staircase which takes you to the dungeon connected below */
} dungeon_cell_t;

/**
 * @struct dungeon_cell
 *
 * @brief Represents a dungeon cell with specific properties.
 *
 * Each dungeon cell has a type defined by `dungeon_cell_t` and an associated hardness value.
 * Hardness can influence traversal difficulty for generating corridors.
 */
typedef struct
{
    dungeon_cell_t type;
    uint8_t hardness;
} dungeon_cell;

/**
 * Room data defining rectangular rooms in the dungeon
 *
 * In cases where `width` are even, `center_x` will be the cell on the left of the two cells that the true center lies on.
 * Similarly with an even `height`, `center_y` will be the top of the two cells that represent the true vertical center of the room.
 */
typedef struct
{
    uint16_t center_x;
    uint16_t center_y;

    uint16_t width;
    uint16_t height;
} dungeon_room_data;

/**
 * @struct dungeon_data
 * @brief Data structure containing all dungeon data and pointers to connecting dungeons.
 *
 * @var cells
 * Two-dimensional array of cells representing the dungeon layout.
 * @var north, east, south, west
 * Pointers to neighboring dungeons in cardinal directions.
 * @var up, down
 * Pointers to dungeons above and below, respectively.
 */
typedef struct dungeon_data
{
    dungeon_cell cells[DUNGEON_WIDTH][DUNGEON_HEIGHT]; /**< Dungeon cell data. Indexed by [x][y] with [0][0] being the top-left corner */
    dungeon_room_data *rooms;                          /**< Array of rooms that are in the dungeon (null-terminated) */

    struct dungeon_data *north; /**< Pointer to the northern dungeon. */
    struct dungeon_data *east;  /**< Pointer to the eastern dungeon. */
    struct dungeon_data *south; /**< Pointer to the southern dungeon. */
    struct dungeon_data *west;  /**< Pointer to the western dungeon. */
    struct dungeon_data *up;    /**< Pointer to the dungeon above. */
    struct dungeon_data *down;  /**< Pointer to the dungeon below. */
} dungeon_data;

typedef enum
{
    DIRECTION_NORTH,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_WEST,
    DIRECTION_UP,
    DIRECTION_DOWN
} world_direction;

/**
 * Displays a dungeon to the command line
 *
 * @param dungeon the dungeon we want to display
 * @param display_border 0=no border, any=draw border on edge of dungeon
 */
void dungeon_display(const dungeon_data *dungeon, const int display_border);

// void dungeon_add(dungeon_data *connection, world_direction connection_direction);
// void dungeon_delete(dungeon_data *dungeon);