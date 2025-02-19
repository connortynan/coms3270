#pragma once

#include <stdio.h>
#include <stdint.h>

#include "common.h"

/**
 * @enum dungeon_cell_t
 *
 * @brief Enumerator of all static cell types, each with special properties
 */
typedef enum dungeon_cell_t
{
    CELL_ROCK = 0,       /**< Solid walls, cannot be traversed over */
    CELL_ROOM = 1,       /**< Room floor */
    CELL_CORRIDOR = 2,   /**< Corridor floor, which connects rooms */
    CELL_STAIR_UP = 3,   /**< Upwards staircase which takes you to the dungeon connected above */
    CELL_STAIR_DOWN = 4, /**< Downwards staircase which takes you to the dungeon connected below */
} dungeon_cell_t;

/**
 * @struct dungeon_cell
 *
 * @brief Represents a dungeon cell with specific properties.
 *
 * Each dungeon cell has a type defined by `dungeon_cell_t` and an associated hardness value.
 * Hardness can influence traversal difficulty for generating corridors.
 */
typedef struct dungeon_cell
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
typedef struct dungeon_room_data
{
    uint8_t center_x;
    uint8_t center_y;

    uint8_t width;
    uint8_t height;
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
    uint16_t num_rooms;                                /**< Number of generated rooms in the dungeon */

    uint8_t pc_x, pc_y;

    struct dungeon_data *north; /**< Pointer to the northern dungeon. */
    struct dungeon_data *east;  /**< Pointer to the eastern dungeon. */
    struct dungeon_data *south; /**< Pointer to the southern dungeon. */
    struct dungeon_data *west;  /**< Pointer to the western dungeon. */
    struct dungeon_data *up;    /**< Pointer to the dungeon above. */
    struct dungeon_data *down;  /**< Pointer to the dungeon below. */
} dungeon_data;

/**
 * @enum world_direction
 *
 * @brief Enumerator for cardinal and vertical directions in the dungeon world.
 */
typedef enum world_direction
{
    DIRECTION_NORTH, /**< Represents the northern direction. */
    DIRECTION_EAST,  /**< Represents the eastern direction. */
    DIRECTION_SOUTH, /**< Represents the southern direction. */
    DIRECTION_WEST,  /**< Represents the western direction. */
    DIRECTION_UP,    /**< Represents the upwards direction. */
    DIRECTION_DOWN   /**< Represents the downwards direction. */
} world_direction;

/**
 * Displays a dungeon to the command line
 *
 * @param dungeon the dungeon we want to display
 * @param display_border 0=no border, any=draw border on edge of dungeon
 */
void dungeon_display(const dungeon_data *dungeon, const int display_border);

// void dungeon_add(dungeon_data *connection, world_direction connection_direction);

/**
 * @brief Destroys a dungeon and frees all allocated resources.
 *
 * This function deallocates any memory associated with the dungeon, including rooms and connections.
 *
 * @param dungeon Pointer to the dungeon to be destroyed.
 * @return Returns 0 on success, or a non-zero value if an error occurs.
 */
int dungeon_destroy(dungeon_data *dungeon);

/**
 * @brief Serializes a dungeon's data to a file.
 *
 * Writes the dungeon's layout, rooms, and metadata to the specified file in a binary format.
 *
 * @param dungeon Pointer to the dungeon to be serialized.
 * @param file File pointer to the file where the dungeon will be written.
 * @return Returns 0 on success, or a non-zero value if an error occurs.
 */
int dungeon_serialize(const dungeon_data *dungeon, FILE *file);

/**
 * @brief Deserializes a dungeon's data from a file.
 *
 * Reads dungeon data from a binary file and populates the provided dungeon structure.
 *
 * @param dungeon Pointer to the dungeon structure to populate with deserialized data.
 * @param file File pointer to the file from which the dungeon will be read.
 * @return Returns 0 on success, or a non-zero value if an error occurs.
 */
int dungeon_deserialize(dungeon_data *dungeon, FILE *file);