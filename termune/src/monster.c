#include "monster.h"

#include "dungeon.h"
#include "game_context.h"
#include "util/pathing.h"
#include "util/vector.h"
#include "util/shadowcast.h"

typedef uint32_t monster_distance_map[DUNGEON_HEIGHT][DUNGEON_WIDTH];

static monster_distance_map tunneling, nontunneling;
static lightmap los_map;

uint64_t _monster_distmap_pathing_cost_eval(size_t node_idx, pathing_context *ctx)
{
    return 1 + (*((uint8_t *)ctx->nodes[node_idx].data)) / 85;
}

void _monster_distmap_pathing_neighbors(size_t node_idx, pathing_context *ctx)
{
    static const int nbor_offsets[8][2] =
        {
            {-1, -1},
            {0, -1},
            {1, -1},
            {-1, 0},
            {1, 0},
            {-1, 1},
            {0, 1},
            {1, 1}};
    static const uint8_t num_nbors = 8;

    uint8_t x = node_idx % DUNGEON_WIDTH;
    uint8_t y = node_idx / DUNGEON_WIDTH;

    uint8_t nx, ny;
    size_t nidx;
    for (uint8_t i = 0; i < num_nbors; i++)
    {
        nx = x + nbor_offsets[i][0];
        ny = y + nbor_offsets[i][1];
        nidx = nx + ny * DUNGEON_WIDTH;
        if (nx < DUNGEON_WIDTH && ny < DUNGEON_HEIGHT && *((uint8_t *)ctx->nodes[nidx].data) < 255)
            pathing_eval_node(node_idx, nidx, ctx);
    }

    monster_distance_map *m = (monster_distance_map *)ctx->data;
    (*m)[y][x] = ctx->nodes[node_idx].cost;
}

int _monster_distmap_pathing_end_condition(size_t node_idx, pathing_context *ctx)
{
    // there is no end condition, we want to visit every grid cell in the map
    return 0;
}

int monster_generate_generic_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon,
    uint8_t *weights,
    uint8_t origin_x, uint8_t origin_y)
{
    // Set all distances to UINT32_MAX (unreachable)
    // Then overwrite the reachable nodes
    memset(dist_map, 0xFF, sizeof(uint32_t) * DUNGEON_WIDTH * DUNGEON_HEIGHT);

    size_t start_idx = origin_x + origin_y * DUNGEON_WIDTH;
    vector *path;
    path = pathing_solve(start_idx, DUNGEON_WIDTH * DUNGEON_HEIGHT,
                         weights, sizeof(uint8_t),
                         dist_map,
                         _monster_distmap_pathing_cost_eval,
                         _monster_distmap_pathing_neighbors,
                         _monster_distmap_pathing_end_condition);
    if (!path)
        return -1;
    vector_destroy(path);
    return 0;
}

static uint8_t *monster_generate_nontunneling_weight_map(dungeon_data *dungeon)
{
    uint8_t *result = (uint8_t *)malloc(sizeof(*result) * DUNGEON_WIDTH * DUNGEON_HEIGHT);

    size_t i;
    uint8_t x, y;

    i = 0;
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            result[i++] = (dungeon->cell_types[y][x] == CELL_ROCK) ? 255 : 0;
        }
    }

    return result;
}

int monster_generate_nontunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon,
    uint8_t origin_x, uint8_t origin_y)
{
    return monster_generate_generic_distance_map(
        dist_map, dungeon,
        monster_generate_nontunneling_weight_map(dungeon), origin_x, origin_y);
}

int monster_generate_tunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon,
    uint8_t origin_x, uint8_t origin_y)
{
    return monster_generate_generic_distance_map(
        dist_map, dungeon,
        &dungeon->cell_hardness[0][0], origin_x, origin_y);
}

monster *monster_init(
    uint8_t x, uint8_t y, uint8_t speed,
    uint8_t intelligence, uint8_t telepathy, uint8_t tunneling, uint8_t erratic)
{
    monster *m = (monster *)malloc(sizeof(*m));

    if (!m)
        return NULL;

    m->alive = 1;
    m->speed = speed;
    m->x = x;
    m->y = y;

    m->characteristics.intelligence = intelligence;
    m->characteristics.telepathy = telepathy;
    m->characteristics.tunneling = tunneling;
    m->characteristics.erratic = erratic;

    m->target_x = UINT8_MAX;
    m->target_y = UINT8_MAX;
    m->has_los = 0;

    return m;
}

int monster_update_telepathic_map(game_context *g)
{
    if (monster_generate_tunneling_distance_map(&tunneling, g->current_dungeon, g->player.x, g->player.y))
        return 1;
    if (monster_generate_nontunneling_distance_map(&nontunneling, g->current_dungeon, g->player.x, g->player.y))
        return 1;
    return 0;
}

int monster_update_los_map(game_context *g)
{
    return shadowcast_solve_lightmap(&los_map, g);
}

uint64_t _monster_pathing_cost_eval(size_t node_idx, pathing_context *ctx)
{
    return ((uint8_t *)ctx->nodes)[node_idx];
}

void _monster_pathing_neighbors(size_t node_idx, pathing_context *ctx)
{
    static const int nbor_offsets[8][2] =
        {
            {-1, -1},
            {0, -1},
            {1, -1},
            {-1, 0},
            {1, 0},
            {-1, 1},
            {0, 1},
            {1, 1}};
    static const uint8_t num_nbors = 8;

    uint8_t x = node_idx % DUNGEON_WIDTH;
    uint8_t y = node_idx / DUNGEON_WIDTH;

    uint8_t nx, ny;
    for (uint8_t i = 0; i < num_nbors; i++)
    {
        nx = x + nbor_offsets[i][0];
        ny = y + nbor_offsets[i][1];
        if (nx < DUNGEON_WIDTH && ny < DUNGEON_HEIGHT)
            pathing_eval_node(node_idx, nx + ny * DUNGEON_WIDTH, ctx);
    }
}

int _monster_pathing_end_condition(size_t node_idx, pathing_context *ctx)
{
    return node_idx == *((size_t *)ctx);
}

/**
 * \ 0000 - .... 0
 * \ 0001 - ...L L
 * \ 0010 - ..t. 0
 * \ 0011 - ..tL L
 * \ 0100 - .T.. L
 * \ 0101 - .T.L L
 * \ 0110 - .Tt. L
 * \ 0111 - .TtL L
 *
 * \ 1000 - I... L
 * \ 1001 - I..L L
 * \ 1010 - I.t. L
 * \ 1011 - I.tL L
 * \ 1100 - IT.. d
 * \ 1101 - IT.L L
 * \ 1110 - ITt. d
 * \ 1111 - ITtL L
 */

static void monster_target_straight_line(monster *m, uint8_t *x, uint8_t *y)
{
    int8_t dx, dy, sx, sy;
    dx = m->target_x - m->x;
    dy = m->target_y - m->y;

    sx = dx < 0 ? -1 : 1;
    sy = dy < 0 ? -1 : 1;

    dx = abs(dx);
    dy = abs(dy);

    if (dy <= dx) // slope < 1, increment x
    {
        *x += sx;
    }
    if (dy >= dx) // slope > 1, increment y
    {
        *y += sy;
    }
}

static void monster_select_target(monster *m, uint8_t *x, uint8_t *y, dungeon_data *d)
{
    *x = m->x;
    *y = m->y;

    if (m->characteristics.erratic)
    {
        // 50% chance to move randomly
        if (rand() & 1)
        {
            *x += (rand() % 3) - 1; // += random integer in range [-1, 1]
            *y += (rand() % 3) - 1; // += random integer in range [-1, 1]
            return;
        } // else continue calculation
    }

    if (m->has_los)
    {
        // direct line
        monster_target_straight_line(m, x, y);
        return;
    }

    if (m->characteristics.intelligence)
    {
        // if monster is telepathic, move based on global distance maps
        if (m->characteristics.telepathy)
        {
            monster_distance_map *dist_map = m->characteristics.tunneling ? &tunneling : &nontunneling;

            // check all 8 neighbors and go towards the minimum cost (start at a random index to randomize same weight cells)
            int nbors[8][2] =
                {
                    {*x - 1, *y - 1},
                    {*x, *y - 1},
                    {*x + 1, *y - 1},
                    {*x - 1, *y},
                    {*x + 1, *y},
                    {*x - 1, *y + 1},
                    {*x, *y + 1},
                    {*x + 1, *y + 1},
                };

            uint8_t i = rand() % 8;
            int min_idx = i;
            int *nbor_pos = &nbors[i][0];
            uint32_t min_dist = (*dist_map)[nbor_pos[1]][nbor_pos[0]];
            uint32_t dist;
            for (uint8_t di = 1; di < 8; di++)
            {
                nbor_pos = &nbors[(i + di) % 8][0];
                dist = (*dist_map)[nbor_pos[1]][nbor_pos[0]];
                if (dist < min_dist)
                {
                    min_idx = (i + di) % 8;
                    min_dist = dist;
                }
            }
            nbor_pos = &nbors[min_idx][0];
            *x = nbor_pos[0];
            *y = nbor_pos[1];
            return;
        }
        else // intelligent, nontele, move towards target if ever seen
        {
            if (m->target_x == UINT8_MAX || m->target_y == UINT8_MAX)
                return; // else stand still
            monster_target_straight_line(m, x, y);
            return;
        }
    }

    if (m->characteristics.telepathy)
    {
        // guarenteed to have a target, follow blindly in a line
        monster_target_straight_line(m, x, y);
    }

    // else dumb nontelepathic monster w/o los, stand still
}

int monster_move(monster *m, game_context *g)
{
    uint8_t desired_x, desired_y;

    // update with new line of sight
    m->has_los = 0;
    if (m->characteristics.telepathy || los_map[m->y][m->x])
    {
        m->target_x = g->player.x;
        m->target_y = g->player.y;

        m->has_los = los_map[m->y][m->x];
    }

    monster_select_target(m, &desired_x, &desired_y, g->current_dungeon);

    if (desired_x == m->x && desired_y == m->y)
        return 0;
    int64_t overlapping_entity = game_entity_id_at(g, desired_x, desired_y);

    if (desired_x < DUNGEON_WIDTH && desired_y < DUNGEON_HEIGHT)
    {
        if (g->current_dungeon->cell_types[desired_y][desired_x] != CELL_ROCK)
        {
            m->x = desired_x;
            m->y = desired_y;
        }
        else if (m->characteristics.tunneling && g->current_dungeon->cell_hardness[desired_y][desired_x] < 255)
        {
            int16_t new_hardness = (int16_t)g->current_dungeon->cell_hardness[desired_y][desired_x] - 85;
            if (new_hardness < 0)
            {
                g->current_dungeon->cell_types[desired_y][desired_x] = CELL_CORRIDOR;
                g->current_dungeon->cell_hardness[desired_y][desired_x] = 0;
                m->x = desired_x;
                m->y = desired_y;
                monster_update_los_map(g);
            }
            else
            {
                g->current_dungeon->cell_hardness[desired_y][desired_x] = new_hardness;
            }
            // recalculate tunneling distance map
            monster_generate_tunneling_distance_map(&tunneling, g->current_dungeon, g->player.x, g->player.y);
        }
        else // can't move
            return 0;
    }
    if (overlapping_entity >= 0 && overlapping_entity < g->num_monsters)
    {
        g->monsters[overlapping_entity].alive = 0;
        return 0;
    }

    if (overlapping_entity == PLAYER_ENTITY_ID)
    {
        g->player.alive = 0;
        return 0;
    }

    return 0;
}

#ifdef DEBUG_DEV_FLAGS
void monster_display_los_map()
{
    shadowcast_display_lightmap(&los_map);
}
#endif