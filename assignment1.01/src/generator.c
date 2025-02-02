#include "generator.h"

int generate_shape_lookup(
    uint16_t center_x, uint16_t center_y,
    uint16_t width, uint16_t height,
    uint16_t *min_x, uint16_t *min_y

);

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