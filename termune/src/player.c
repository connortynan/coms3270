#include "player.h"

#include "monster.h"
#include "game_context.h"

#include <stdlib.h>

#define DEFAULT_PLAYER_SPEED 10

player *player_init(uint8_t x, uint8_t y)
{
    player *p = (player *)malloc(sizeof(player));
    if (!p)
        return NULL; // Memory allocation failed

    p->x = x;
    p->y = y;
    p->speed = DEFAULT_PLAYER_SPEED;
    p->alive = 1;

    return p;
}

int player_handle_move(player *p, game_context *g)
{
    uint8_t target_x = p->x;
    uint8_t target_y = p->y;

    target_x += (rand() % 3) - 1; // += random integer in range [-1, 1]
    target_y += (rand() % 3) - 1; // += random integer in range [-1, 1]

    if (target_x == p->x && target_y == p->y)
        return 0;
    int64_t overlapping_entity = game_entity_id_at(g, target_x, target_y);

    if (g->current_dungeon->cell_types[target_y][target_x] != CELL_ROCK)
    {
        p->x = target_x;
        p->y = target_y;
    }
    else
    {
        return 0;
    }

    if (overlapping_entity >= 0 && overlapping_entity < g->num_monsters)
    {
        g->monsters[overlapping_entity].alive = 0;
    }

    monster_update_telepathic_map(g);
    monster_update_los_map(g);

    return 0;
}
