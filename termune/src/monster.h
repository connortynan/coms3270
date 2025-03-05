#pragma once

#include <stdint.h>

#include "common.h"
#include "dungeon.h"
#include "util/vector.h"

/**
 * @brief Represents a monster entity in the dungeon.
 *
 * Each monster has movement characteristics, speed, and a target position.
 * The monster's behavior is determined by its characteristics, which are
 * stored as bitfields and accessible as a single flag value.
 */
typedef struct monster
{
    uint8_t x;        /**< Current x-coordinate of the monster. */
    uint8_t y;        /**< Current y-coordinate of the monster. */
    uint8_t speed;    /**< Movement speed of the monster (range: 5-20). */
    uint8_t alive;    /**< Alive status: 1 = alive, 0 = dead. */
    uint8_t has_los;  /**< Monster currently can see the player */
    uint8_t target_x; /**< Target x-coordinate (for pathfinding). */
    uint8_t target_y; /**< Target y-coordinate (for pathfinding). */

    /**
     * @union characteristics
     * @brief Stores the monster's characteristics as individual bitfields and as a single flag value.
     *
     * The bitfields define the monster's behavior:
     * - **Intelligence** (bit 0): Uses shortest pathfinding if enabled.
     * - **Telepathy** (bit 1): Always knows the PC's location if enabled.
     * - **Tunneling** (bit 2): Can dig through walls if enabled.
     * - **Erratic** (bit 3): Moves randomly 50% of the time if enabled.
     */
    union
    {
        struct
        {
            uint8_t intelligence : 1; /**< 1 if intelligent (follows shortest path), 0 otherwise. */
            uint8_t telepathy : 1;    /**< 1 if telepathic (always knows PC location), 0 otherwise. */
            uint8_t tunneling : 1;    /**< 1 if can tunnel through walls, 0 otherwise. */
            uint8_t erratic : 1;      /**< 1 if moves randomly 50% of the time, 0 otherwise. */
        };
        uint8_t flags : 4; /**< Bit-packed representation of characteristics. */
    } characteristics;

} monster;

monster *monster_init(
    uint8_t x, uint8_t y, uint8_t speed,
    uint8_t intelligence, uint8_t telepathy, uint8_t tunneling, uint8_t erratic);

int monster_update_telepathic_map(dungeon_data *d);
int monster_update_los_map(dungeon_data *d);

int monster_move(monster *m, dungeon_data *d);