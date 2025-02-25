#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#include "dungeon.h"
#include "generator.h"
#include "monster.h"
#include "util/noise.h"

int main(int argc, char const *argv[])
{
    char filename[128];
    sprintf(filename, "%s/.rlg327/dungeon", getenv("HOME"));

    // Check if the directory exists, if not, create it
    char dir[128];
    snprintf(dir, sizeof(dir), "%s/.rlg327", getenv("HOME"));

    struct stat st = {0};
    if (stat(dir, &st) == -1)
    {
        if (mkdir(dir, 0700) == -1)
        {
            perror("Error creating directory");
            return 1;
        }
        printf("Directory created: %s\n", dir);
    }

    int save_flag = 0;
    int load_flag = 0;

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        if (!strcmp("--save", arg))
            save_flag = 1;
        if (!strcmp("--load", arg))
            load_flag = 1;
    }

    dungeon_data dungeon;

    // Get the dungeon (either from file or random seed)
    if (load_flag)
    {
        printf("Reading from file: %s\n", filename);
        FILE *dungeon_load = fopen(filename, "rb");
        dungeon_deserialize(&dungeon, dungeon_load);
        fclose(dungeon_load);
    }
    else
    {
        int rng_seed = time(NULL);

        srand(rng_seed);
        noise_generate_permutation();

        generator_parameters params = {
            .min_num_rooms = 6,
            .max_num_rooms = 10,

            .min_room_width = 6,
            .max_room_width = 20,

            .min_room_height = 4,
            .max_room_height = 10,

            .min_num_stairs = 2,
            .max_num_stairs = 4,

            .min_rock_hardness = 128,
            .max_rock_hardness = 192,

            .rock_hardness_smoothness = 5,
            .rock_hardness_noise_amount = 500.f,
        };

        printf("Generating from seed: %d\n", rng_seed);
        generator_generate_dungeon(&dungeon, &params);
    }

    // Calculate distance maps and display dungeon/maps
    printf("========================================"
           "========================================\n");
    dungeon_display(&dungeon, 0);

    monster_distance_map nontunneling;
    monster_distance_map tunneling;
    monster_generate_nontunneling_distance_map(&nontunneling, &dungeon);
    monster_generate_tunneling_distance_map(&tunneling, &dungeon);

    printf("========================================"
           "========================================\n");
    monster_print_distance_map(&nontunneling, &dungeon);
    printf("========================================"
           "========================================\n");
    monster_print_distance_map(&tunneling, &dungeon);
    printf("========================================"
           "========================================\n");

    if (save_flag)
    {
        printf("Saving to file: %s\n", filename);
        FILE *dungeon_save = fopen(filename, "wb");
        dungeon_serialize(&dungeon, dungeon_save);
        fclose(dungeon_save);
    }

    dungeon_destroy(&dungeon);

    return 0;
}