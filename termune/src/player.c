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

int player_move(game_context *g, int dx, int dy)
{
    uint8_t target_x = g->player.x;
    uint8_t target_y = g->player.y;

    target_x += dx;
    target_y += dy;

    if (target_x == g->player.x && target_y == g->player.y)
        return 0;
    int64_t overlapping_entity = game_entity_id_at(g, target_x, target_y);

    if (g->current_dungeon->cell_types[target_y][target_x] != CELL_ROCK)
    {
        g->player.x = target_x;
        g->player.y = target_y;
    }
    else
    {
        return 0;
    }

    if (overlapping_entity >= 0 && overlapping_entity < g->num_monsters)
    {
        g->monsters[overlapping_entity].alive = 0;
        g->alive_monsters--;
    }

    monster_update_telepathic_map(g);
    monster_update_los_map(g);

    return 0;
}
