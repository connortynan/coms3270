#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dungeon.h"
#include "generator.h"
#include "util/noise.h"

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    noise_generate_permutation();

    dungeon_data dungeon;

    generator_parameters params = {
        .min_num_rooms = 6,
        .max_num_rooms = 8,

        .min_room_width = 6,
        .max_room_width = 12,

        .min_room_height = 4,
        .max_room_height = 10,

        .min_num_stairs = 0,
        .max_num_stairs = 0,
    };

    generator_generate_dungeon(&dungeon, &params);

    // dungeon_display(&dungeon, 0);

    dungeon_data noisy;

    for (int y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (int x = 0; x < DUNGEON_WIDTH; x++)
        {
            noisy.cells[x][y].hardness = (noise_perlin(0.2f * x, 0.2f * y) + 1.f) * 255;
            noisy.cells[x][y].type = noisy.cells[x][y].hardness / 60;
        }
    }
    dungeon_display(&noisy, 1);

    return 0;
}
