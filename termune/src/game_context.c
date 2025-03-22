#include "game_context.h"

#include <stdlib.h>
#include "monster.h"
#include "generator.h"

/**
 * @brief Comparison function for event queue (min-heap).
 *
 * Orders events by `turn_id`, ensuring earlier events execute first.
 *
 * @param a Pointer to the first event.
 * @param b Pointer to the second event.
 * @param context Unused, but included for compatibility with heap structure.
 * @return Negative if `a` occurs before `b`, positive if `b` occurs before `a`, and zero if equal.
 */
static int event_queue_cmp(const void *a, const void *b, void *context)
{
    (void)context; // Unused parameter
    const game_event *event_a = (const game_event *)a;
    const game_event *event_b = (const game_event *)b;

    if (event_a->turn_id < event_b->turn_id)
        return -1;
    if (event_a->turn_id > event_b->turn_id)
        return 1;
    if (event_a->entity_id > event_b->entity_id)
        return -1;
    return 0;
}

game_context *game_init(dungeon_data *dungeon, int64_t num_monsters, uint8_t pc_x, uint8_t pc_y)
{
    game_context *g = (game_context *)malloc(sizeof(game_context));
    if (!g)
        return NULL; // Memory allocation failed

    g->current_dungeon = dungeon;
    if (!g->current_dungeon)
    {
        free(g);
        return NULL;
    }

    g->player.x = pc_x;
    g->player.y = pc_y;
    g->player.speed = 10;
    g->player.alive = 1;

    g->monsters = (monster *)malloc(sizeof(monster) * num_monsters);
    if (!g->monsters)
    {
        dungeon_destroy(g->current_dungeon);
        free(g);
        return NULL; // Monster allocation failed
    }

    for (int i = 0; i < num_monsters; i++)
    {
        uint8_t monster_x, monster_y;
        do
        {
            monster_x = rand() % DUNGEON_WIDTH;
            monster_y = rand() % DUNGEON_HEIGHT;
        } while (g->current_dungeon->cell_types[monster_y][monster_x] == CELL_ROCK);

        g->monsters[i].speed = (rand() % 15) + 5;
        g->monsters[i].characteristics.flags = rand() & 0xF;
        g->monsters[i].x = monster_x;
        g->monsters[i].y = monster_y;
        g->monsters[i].target_x = UINT8_MAX;
        g->monsters[i].target_y = UINT8_MAX;
        g->monsters[i].has_los = 0;
        g->monsters[i].alive = 1;
    }

    g->event_queue = heap_init(0, sizeof(game_event), event_queue_cmp, NULL);
    if (!g->event_queue)
    {
        dungeon_destroy(g->current_dungeon);
        free(g);
        return NULL; // Event queue allocation failed
    }

    g->num_monsters = num_monsters;
    g->alive_monsters = num_monsters;
    g->turn_id = 0; // Start game at turn 0
    g->running = 1; // Game starts running

    return g;
}

int run_game_event(game_context *g, int64_t entity_id)
{
    // Copy event and add delay inside processing
    game_event next_event = {
        .entity_id = entity_id,
        .turn_id = g->turn_id};

    if (entity_id >= 0 && entity_id < g->num_monsters)
    {
        if (!g->monsters[entity_id].alive)
            return 0;

        monster_move(&g->monsters[entity_id], g);
        next_event.turn_id += 1000 / g->monsters[entity_id].speed;
    }
    else
    {
        return 1;
    }

    heap_insert(g->event_queue, &next_event);
    return 0;
}

int game_process_events(game_context *g)
{
    if (!g || !g->event_queue)
        return -1;

    game_event *event = heap_peek(g->event_queue);
    while (event && event->turn_id <= g->turn_id)
    {
        // Only remove from queue if actually processed this turn
        heap_poll(g->event_queue, event);
        run_game_event(g, event->entity_id);
        event = heap_peek(g->event_queue);
    }

    g->turn_id += 10;
    return 0;
}

int game_destroy(game_context *g)
{
    if (!g)
        return -1;

    if (g->event_queue)
        heap_destroy(g->event_queue);

    if (g->current_dungeon)
        dungeon_destroy(g->current_dungeon);

    if (g->monsters)
        free(g->monsters);

    free(g);
    return 0;
}

int64_t game_entity_id_at(game_context *g, uint8_t x, uint8_t y)
{
    if (x >= DUNGEON_WIDTH || y >= DUNGEON_HEIGHT)
        return -1;

    if (x == g->player.x && y == g->player.y)
        return PLAYER_ENTITY_ID;

    int64_t i;
    monster m;
    for (i = 0; i < g->num_monsters; i++)
    {
        m = g->monsters[i];
        if (m.alive && x == m.x && y == m.y)
            return i;
    }
    return -1;
}

void game_generate_display_buffer(const game_context *g, char *result)
{
    static const char hex_char_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    dungeon_generate_display_buffer(g->current_dungeon, result);

    size_t i;
    monster m;
    for (i = 0; i < g->num_monsters; i++)
    {
        m = g->monsters[i];
        if (m.alive)
            result[m.x + m.y * DUNGEON_WIDTH] = hex_char_map[m.characteristics.flags];
    }

    if (g->player.alive)
        result[g->player.x + g->player.y * DUNGEON_WIDTH] = '@';
}

void game_display(const game_context *g, const int display_border)
{
    uint8_t x, y;

    char output[DUNGEON_WIDTH * DUNGEON_HEIGHT];

    game_generate_display_buffer(g, output);

    if (display_border)
    {
        // Top border
        output[0] = '/';
        for (x = 1; x < DUNGEON_WIDTH - 1; x++)
            output[x] = '-';
        output[DUNGEON_WIDTH - 1] = '\\';

        // Side borders
        for (y = 1; y < DUNGEON_HEIGHT - 1; y++)
        {
            output[y * DUNGEON_WIDTH] = '|';
            output[(DUNGEON_WIDTH - 1) + y * DUNGEON_WIDTH] = '|';
        }

        // Bottom border
        output[(DUNGEON_HEIGHT - 1) * DUNGEON_WIDTH] = '\\';
        for (x = 1; x < DUNGEON_WIDTH - 1; x++)
            output[x + (DUNGEON_HEIGHT - 1) * DUNGEON_WIDTH] = '-';
        output[(DUNGEON_WIDTH - 1) + (DUNGEON_HEIGHT - 1) * DUNGEON_WIDTH] = '/';
    }
    // Display output
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
#ifdef DEBUG_DEV_FLAGS
            // Print hardness by color
            printf("\033[48;2;%d;127;127m%c\033[0m", g->current_dungeon->cell_hardness[y][x], output[x + y * DUNGEON_WIDTH]);
#else
            printf("%c", output[x + y * DUNGEON_WIDTH]);
#endif // DEBUG_DEV_FLAG
        }
        printf("\n");
    }
}