#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

#include "game_context.hpp"
#include "ui.hpp"
#include "util/noise.hpp"
#include "util/fs.hpp"
#include "util/colors.hpp"

constexpr mapsize_t DUNGEON_WIDTH = 80;
constexpr mapsize_t DUNGEON_HEIGHT = 21;

int main(int argc, char const *argv[])
{
    // Handle CLI args
    bool save = false, load = false;
    int num_mon = 10;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--save")
            save = true;
        else if (arg == "--load")
            load = true;
        else if (arg == "--nummon" && i + 1 < argc)
            num_mon = std::max(1, atoi(argv[++i]));
    }

    // Init RNG, noise, and file system
    fs::ensure_data_dir_exists();
    Noise::generate_permutation(time(nullptr));
    init_color_pairs(COLOR_BLACK);

    std::string filename = fs::join(fs::rlg327_data_dir(), "dungeon");

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
        .rock_hardness_noise_amount = 50.f};

    // Initialize core components
    GameContext game(params, DUNGEON_WIDTH, DUNGEON_HEIGHT, num_mon, time(nullptr));
    ui::Context ui(game, 10.f);
    game.player.ui = &ui;

    // Load or generate dungeon
    if (load)
    {
        std::ifstream infile(filename, std::ios::binary);
        mapsize_t x = 0, y = 0;
        if (!infile)
        {
            std::cerr << "Failed to load dungeon.\n";
            return 1;
        }
        Dungeon d = Dungeon::deserialize(infile, x, y);
        game.set_dungeon(d, x, y);
    }
    else
    {
        game.regenerate_dungeon();
    }

    if (save)
    {
        std::ofstream outfile(filename, std::ios::binary);
        if (outfile)
            game.dungeon.serialize(outfile, game.player.x, game.player.y);
    }

    // Schedule all character events
    game.schedule_character_event(&game.player);
    for (auto *m : game.filtered<Character>())
    {
        if (m != &game.player)
            game.schedule_character_event(m);
    }

    // Show title and run
    ui.display_title();
    getch();
    ui.display_message("Spawned at (%d, %d)", game.player.x, game.player.y);

    game.update_on_change();
    ui.update_game_window();

    while (game.running && ui.running)
    {
        game.process_events();
    }

    if (ui.running)
    {
        ui.display_message(game.player.active ? "YOU WIN! (Press any key)" : "YOU LOSE. (Press any key)");
        getch();
    }

    return 0;
}
