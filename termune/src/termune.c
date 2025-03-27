#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#include "dungeon.h"
#include "generator.h"
#include "monster.h"
#include "game_context.h"
#include "ui.h"
#include "util/noise.h"
#include "util/heap.h"

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
    }

    int save_flag = 0;
    int load_flag = 0;
    int num_mon = 10; // Default number of monsters

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        if (!strcmp("--save", arg))
            save_flag = 1;
        else if (!strcmp("--load", arg))
            load_flag = 1;
        else if (!strcmp("--nummon", arg) && (i + 1) < argc)
        {
            num_mon = atoi(argv[i + 1]);
            if (num_mon <= 0)
            {
                num_mon = 10;
            }
            i++;
        }
    }

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
        .rock_hardness_noise_amount = 50.f,
    };

    int rng_seed = time(NULL);

    srand(rng_seed);
    noise_generate_permutation();

    game_context *game = game_init(num_mon, &params);

    // Get the dungeon (either from file or random seed)
    if (load_flag)
    {
        dungeon_data *dungeon = (dungeon_data *)malloc(sizeof(*dungeon));
        uint8_t pc_x, pc_y;

        FILE *dungeon_load = fopen(filename, "rb");
        dungeon_deserialize(dungeon, dungeon_load, &pc_x, &pc_y);
        fclose(dungeon_load);

        game_set_dungeon(game, dungeon, pc_x, pc_y);
    }
    else
    {
        game_regenerate_dungeon(game);
    }

    if (save_flag)
    {
        FILE *dungeon_save = fopen(filename, "wb");
        dungeon_serialize(game->current_dungeon, dungeon_save, game->player.x, game->player.y);
        fclose(dungeon_save);
    }

    fflush(stdout);
    ui_context *ui = ui_init();

    // begin the queue for every monster
    game_event event = {};
    for (int i = 0; i < game->num_monsters; i++)
    {
        event.entity_id = i;
        event.turn_id = 1000 / game->monsters[i].speed;
        heap_insert(game->event_queue, &event);
    }

    ui_display_title(ui);
    getch();
    UI_MESSAGE(ui, "Spawned at (%d, %d)", game->player.x, game->player.y);

    ui_display_game(ui, game);
    while (game->running && ui->running)
    {
        ui_handle_player_input(ui, game);
        ui_display_game(ui, game);
    }

    if (ui->running)
    {
        UI_MESSAGE(ui, "%s (Click any button to exit)", game->player.alive ? "YOU WIN!" : "YOU LOSE.");
        getch();
    }

    ui_shutdown(ui);
    game_destroy(game);

    return 0;
}