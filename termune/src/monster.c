#include "monster.h"

#include "dungeon.h"
#include "util/pathing.h"
#include "util/vector.h"
#include "util/shadowcast.h"

typedef uint32_t monster_distance_map[DUNGEON_WIDTH][DUNGEON_HEIGHT];

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
    (*m)[x][y] = ctx->nodes[node_idx].cost;
}

int _monster_distmap_pathing_end_condition(size_t node_idx, pathing_context *ctx)
{
    // there is no end condition, we want to visit every grid cell in the map
    return 0;
}

int monster_generate_generic_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon,
    uint8_t *weights)
{
    // Set all distances to UINT32_MAX (unreachable)
    // Then overwrite the reachable nodes
    memset(dist_map, 0xFF, sizeof(uint32_t) * DUNGEON_WIDTH * DUNGEON_HEIGHT);

    size_t start_idx = dungeon->pc_x + dungeon->pc_y * DUNGEON_WIDTH;
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
            result[i++] = (dungeon->cell_types[x][y] == CELL_ROCK) ? 255 : 0;
        }
    }

    return result;
}

int monster_generate_nontunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon)
{
    return monster_generate_generic_distance_map(dist_map, dungeon, monster_generate_nontunneling_weight_map(dungeon));
}

int monster_generate_tunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon)
{
    return monster_generate_generic_distance_map(dist_map, dungeon, &dungeon->cell_hardness[0][0]);
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

int monster_update_telepathic_map(dungeon_data *d)
{
    if (monster_generate_tunneling_distance_map(&tunneling, d))
        return 1;
    if (monster_generate_nontunneling_distance_map(&nontunneling, d))
        return 1;
    return 0;
}

int monster_update_los_map(dungeon_data *d)
{
    return shadowcast_solve_lightmap(&los_map, d);
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
    size_t nidx;
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
 * 0000 - .... 0
 * \ 0001 - ...L L
 * 0010 - ..t. 0
 * \ 0011 - ..tL L
 * \ 0100 - .T.. L
 * \ 0101 - .T.L L
 * \ 0110 - .Tt. L
 * \ 0111 - .TtL L
 *
 * \ 1000 - I... L
 * \ 1001 - I..L L
 * \ 1010 - I.t. p
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
            uint32_t min_dist = (*dist_map)[nbor_pos[0]][nbor_pos[1]];
            uint32_t dist;
            for (uint8_t di = 1; di < 8; di++)
            {
                nbor_pos = &nbors[(i + di) % 8][0];
                dist = (*dist_map)[nbor_pos[0]][nbor_pos[1]];
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
        else if (m->characteristics.tunneling) // non-telepathic, intelligent, nontunneling: recalculate path
        {
            size_t goal_idx = m->target_x + m->target_y * DUNGEON_WIDTH;

            vector *path = pathing_solve(m->x + m->y * DUNGEON_WIDTH, DUNGEON_WIDTH * DUNGEON_HEIGHT,
                                         &d->cell_hardness[0][0], sizeof(uint8_t), &goal_idx,
                                         _monster_pathing_cost_eval, _monster_pathing_neighbors, _monster_pathing_end_condition);

            if (!path)
                return;
            size_t *next_idx = vector_at(path, 1);
            *x = *next_idx % DUNGEON_WIDTH;
            *y = *next_idx / DUNGEON_WIDTH;
            return;
        }
        else // intelligent, nontele w/o tunneling, move towards target if ever seen
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

int monster_move(monster *m, dungeon_data *d)
{
    uint8_t desired_x, desired_y;

    // update with new line of sight
    m->has_los = 0;
    if (m->characteristics.telepathy || los_map[m->x][m->y])
    {
        m->target_x = d->pc_x;
        m->target_y = d->pc_y;

        m->has_los = los_map[m->x][m->y];
    }

    monster_select_target(m, &desired_x, &desired_y, d);

    if (d->cell_types[desired_x][desired_y] != CELL_ROCK)
    {
        m->x = desired_x;
        m->y = desired_y;
    }
    else if (m->characteristics.tunneling)
    {
        int16_t new_hardness = (int16_t)d->cell_hardness - 85;
        if (new_hardness < 0)
        {
            d->cell_types[desired_x][desired_y] = CELL_CORRIDOR;
            d->cell_hardness[desired_x][desired_y] = 0;
            m->x = desired_x;
            m->y = desired_y;
        }
        else
        {
            d->cell_hardness[desired_x][desired_y] = new_hardness;
        }
        // recalculate tunneling distance map
        monster_generate_tunneling_distance_map(&tunneling, d);
    }

    // kill overlapping monsters
}