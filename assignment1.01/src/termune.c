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
        .max_room_width = 20,

        .min_room_height = 4,
        .max_room_height = 10,

        .min_num_stairs = 0,
        .max_num_stairs = 0,

        .min_rock_hardness = 196,
        .max_rock_hardness = 234,

        .rock_hardness_noise_amount = 10.f,
    };

    generator_generate_dungeon(&dungeon, &params);

    dungeon_display(&dungeon, 0);

    dungeon_destroy(&dungeon);

    return 0;
}