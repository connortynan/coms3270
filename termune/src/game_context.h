#pragma once

#include "dungeon.h"
#include "player.h"
#include "monster.h"
#include "util/heap.h"

#define PLAYER_ENTITY_ID INT64_MAX

typedef struct game_event
{
    int64_t entity_id;
    uint64_t turn_id;
} game_event;

/**
 * @struct game_context
 * @brief Represents the current state of the game, including the dungeon, player, and game entities.
 */
typedef struct game_context
{
    dungeon_data *current_dungeon; /**< Pointer to the current dungeon data. */

    player player; /**< Pointer to the player character. */

    monster *monsters; /**< Array holding `monster` structures representing enemies in the game. */
    int64_t num_monsters;
    int64_t alive_monsters;

    uint8_t running; /**< Game running status: 1 = running, 0 = stopped. */

    heap *event_queue; /**< Priority queue for managing game events. */

    uint64_t turn_id; /**< Current turn id for processing event queue */
} game_context;

/**
 * @brief Initializes the game context.
 *
 * Allocates and initializes the game context, including the dungeon, player, and monsters.
 *
 * @param dungeon The starting dungeon, pregenerated
 * @param num_monsters The number of monsters to generate in the game.
 * @return A pointer to the initialized game context, or NULL on failure.
 */
game_context *game_init(dungeon_data *dungeon, int64_t num_monsters, uint8_t pc_x, uint8_t pc_y);

int game_add_event(size_t entity_id, uint64_t turn_id);

/**
 * @brief Processes game events.
 *
 * Handles events in the event queue, updating the game state accordingly.
 *
 * @param g Pointer to the game context.
 * @return Returns 0 on success, or an error code on failure.
 */
int game_process_events(game_context *g);

/**
 * @brief Cleans up and deallocates game resources.
 *
 * Frees all allocated memory associated with the game context, including the dungeon, player, and monsters.
 *
 * @param g Pointer to the game context.
 * @return Returns 0 on success, or an error code on failure.
 */
int game_destroy(game_context *g);

/**
 * @brief Retrieves the entity ID at a specific position in the game world.
 *
 * This function checks if an entity (player, monster, or other object) exists at the given
 * (x, y) coordinate in the game world.
 *
 * @param g Pointer to the game context.
 * @param x The x-coordinate to check.
 * @param y The y-coordinate to check.
 * @return The entity ID at the specified position, or -1 if no entity is present.
 */
int64_t game_entity_id_at(game_context *g, uint8_t x, uint8_t y);

void game_generate_display_buffer(const game_context *g, char *result);

void game_display(const game_context *g, const int display_border);
