#pragma once

#include "common.h"

/**
 * @enum dungeon_cell
 *
 * @brief Enumerator of all static cell states, each with special properties
 */
typedef enum
{
    ROCK,      /**< Solid walls, cannot be traversed over */
    ROOM,      /**< Room floor */
    CORRIDOR,  /**< Corridor floor, which connects rooms */
    STAIR_UP,  /**< Upwards staircase which takes you to the dungeon connected above */
    STAIR_DOWN /**< Downwards staircase which takes you to the dungeon connected below */
} dungeon_cell;

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
typedef struct
{
    dungeon_cell cells[DUNGEON_WIDTH][DUNGEON_HEIGHT]; /**< Dungeon cell data. Indexed by [x][y] with [0][0] being the top-left corner */

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
 * @param display_border 0=no border, any=draw_border on edge of dungeon
 */
void dungeon_display(dungeon_data *dungeon, int display_border);

// void dungeon_add(dungeon_data *connection, world_direction connection_direction);