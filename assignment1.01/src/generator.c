#include "generator.h"

int generator_generate_dungeon(dungeon_data *dungeon, generator_parameters *params)
{
    int x, y;

    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            dungeon->cells[x][y] = (x == 0 || x == DUNGEON_WIDTH - 1 || y == 0 || y == DUNGEON_HEIGHT - 1) ? 0 : (rand() % 5);
        }
    }

    return 0;
}