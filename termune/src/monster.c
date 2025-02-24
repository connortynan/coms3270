#include "monster.h"

#include "dungeon.h"
#include "util/pathing.h"
#include "util/vector.h"

uint64_t _monster_pathing_cost_eval(size_t node_idx, pathing_context *ctx)
{
    return 1 + (*((uint8_t *)ctx->nodes[node_idx].data)) / 85;
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
        nidx = nx + ny * DUNGEON_WIDTH;
        if (nx < DUNGEON_WIDTH && ny < DUNGEON_HEIGHT && *((uint8_t *)ctx->nodes[nidx].data) < 255)
            pathing_eval_node(node_idx, nidx, ctx);
    }

    monster_distance_map *m = (monster_distance_map *)ctx->data;
    m->distances[x][y] = ctx->nodes[node_idx].cost;
}

int _monster_pathing_end_condition(size_t node_idx, pathing_context *ctx)
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
    memset(dist_map->distances, 0xFF, sizeof(uint32_t) * DUNGEON_WIDTH * DUNGEON_HEIGHT);

    size_t start_idx = dungeon->pc_x + dungeon->pc_y * DUNGEON_WIDTH;
    vector *path;
    path = pathing_solve(start_idx, DUNGEON_WIDTH * DUNGEON_HEIGHT,
                         weights, sizeof(uint8_t),
                         dist_map,
                         _monster_pathing_cost_eval,
                         _monster_pathing_neighbors,
                         _monster_pathing_end_condition);
    if (!path)
        return -1;
    vector_destroy(path);
    return 0;
}

int monster_generate_nontunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon)
{
    uint8_t weights[DUNGEON_WIDTH * DUNGEON_HEIGHT];
    size_t i;
    uint8_t x, y;

    i = 0;
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            weights[i++] = (dungeon->cells[x][y].type == CELL_ROCK) ? 255 : 0;
        }
    }

    return monster_generate_generic_distance_map(dist_map, dungeon, weights);
}

int monster_generate_tunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon)
{
    uint8_t weights[DUNGEON_WIDTH * DUNGEON_HEIGHT];
    size_t i;
    uint8_t x, y;

    i = 0;
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            weights[i++] = dungeon->cells[x][y].hardness;
        }
    }

    return monster_generate_generic_distance_map(dist_map, dungeon, weights);
}

int monster_print_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon)
{
    uint8_t x, y;
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            if (x == dungeon->pc_x && y == dungeon->pc_y)
                printf("@");
            else if (dist_map->distances[x][y] == UINT32_MAX)
                printf(" ");
            else
                printf("%1d", dist_map->distances[x][y] % 10);
        }
        printf("\n");
    }
    return 0;
}