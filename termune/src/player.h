#pragma once

#include <stdint.h>

typedef struct game_context game_context;

/**
 * @struct player
 * @brief Represents the player character in the game.
 */
typedef struct player
{
    uint8_t x;     /**< X-coordinate of the player on the game grid. */
    uint8_t y;     /**< Y-coordinate of the player on the game grid. */
    uint8_t speed; /**< Movement speed of the player. */
    uint8_t alive; /**< Alive status: 1 = alive, 0 = dead. */
} player;

/**
 * @brief Initializes a player instance with the given position.
 *
 * @param x The starting X-coordinate of the player.
 * @param y The starting Y-coordinate of the player.
 * @return A pointer to the initialized player structure.
 */
player *player_init(uint8_t x, uint8_t y);

/**
 * @brief Handles player movement based on input or game logic.
 * @param p Pointer to player who is moving.
 * @param p Pointer to game context containing the player.
 *
 * @return 0 on success, 1 otherwise.
 */
int player_handle_move(player *p, game_context *g);
