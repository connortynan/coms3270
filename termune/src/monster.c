#include "monster.h"

int monster_generate_nontunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon)
{
    return 0;
}

int monster_generate_tunneling_distance_map(
    monster_distance_map *dist_map,
    dungeon_data *dungeon)
{
    return 0;
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