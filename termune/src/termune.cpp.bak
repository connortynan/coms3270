#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "game_context.h"
#include "ui.h"
#include "util/noise.h"

constexpr mapsize_t DUNGEON_WIDTH = 80;
constexpr mapsize_t DUNGEON_HEIGHT = 21;

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

    Dungeon::Generator::Parameters params = {
        .min_room_width = 6,
        .max_room_width = 20,

        .min_room_height = 4,
        .max_room_height = 10,

        .min_num_rooms = 6,
        .max_num_rooms = 10,

        .min_num_stairs = 2,
        .max_num_stairs = 4,

        .min_rock_hardness = 128,
        .max_rock_hardness = 192,

        .rock_hardness_smoothness = 5,
        .rock_hardness_noise_amount = 50.f,
    };

    Noise::generate_permutation(time(NULL));

    GameContext game(params, num_mon, DUNGEON_WIDTH, DUNGEON_HEIGHT);

    // Get the dungeon (either from file or random seed)
    if (load_flag)
    {
        std::ifstream infile(filename, std::ios::binary);
        if (!infile)
        {
            std::cerr << "Failed to open dungeon file for loading: " << filename << "\\n";
            return 1;
        }

        mapsize_t pc_x, pc_y;
        Dungeon dungeon = Dungeon::deserialize(infile, pc_x, pc_y);
        game.set_dungeon(dungeon, pc_x, pc_y);
    }
    else
    {
        game.regenerate_dungeon();
    }

    // Save dungeon to file if specifiec
    if (save_flag)
    {
        std::ofstream outfile(filename, std::ios::binary);
        if (!outfile)
        {
            std::cerr << "Failed to open dungeon file for saving: " << filename << "\\n";
            return 1;
        }

        game.dungeon.serialize(outfile, game.player->x, game.player->y);
    }

    fflush(stdout);
    UiManager ui(game);

    // begin the queue for every monster
    for (const auto &m : game.alive_monsters)
    {
        game.add_entity_event(m->id, 1000 / m->speed);
    }
    // add player event to the queue
    game.add_entity_event(ENTITY_PLAYER, 1000 / game.player->speed);

    ui.display_title();
    getch();
    ui.display_message("Spawned at (%d, %d)", game.player->x, game.player->y);

    game.update_on_change();
    ui.update_game_window();
    while (game.running && ui.running)
    {
        ui.handle_player_input();
        ui.update_game_window();
    }

    if (ui.running)
    {
        ui.display_message("%s (Click any button to exit)", game.player->alive ? "YOU WIN!" : "YOU LOSE.");
        getch();
    }

    return 0;
}