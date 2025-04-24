#pragma once

#ifdef USE_WNCURSES
#include <ncursesw/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <cstdint>
#include <memory>
#include <array>
#include <string>
#include "types.hpp"
#include "game_context.hpp"
#include "ui_command.hpp"
#include "object_item.hpp"

namespace ui
{

#define CENTER_WINDOWS_Y(out_h, in_h) (((out_h) - (in_h)) / 2)
#define CENTER_WINDOWS_X(out_w, in_w) (((out_w) - (in_w)) / 2)

    class Context
    {
    public:
        explicit Context(GameContext &game, float target_fps = 5.f);
        ~Context();

        static constexpr int MESSAGE_HEIGHT = 1;
        static constexpr int STATUS_HEIGHT = 2;

        static constexpr int DUNGEON_WIN_WIDTH = 80;
        static constexpr int DUNGEON_WIN_HEIGHT = 21;

        static constexpr int MONSTER_WIN_WIDTH = 30;
        static constexpr int MONSTER_WIN_HEIGHT = 10;

        static constexpr int EQUIPMENT_WIN_WIDTH = 40;
        static constexpr int EQUIPMENT_WIN_HEIGHT = 14;

        static constexpr int INVENTORY_WIN_WIDTH = 40;
        static constexpr int INVENTORY_WIN_HEIGHT = 12;

        static constexpr int LORE_WIN_WIDTH = 80;
        static constexpr int LORE_WIN_HEIGHT = 18;

        enum class WindowID
        {
            MESSAGE,
            DUNGEON,
            MONSTER,
            EQUIPMENT,
            INVENTORY,
            LORE,
            STATUS,
            COUNT
        };

        // [WindowID][h,w,y,x])
        static constexpr int WINDOW_SIZE[static_cast<size_t>(WindowID::COUNT)][4] =
            {
                {1, DUNGEON_WIN_WIDTH, 0, 0}, // MESSAGE

                {DUNGEON_WIN_HEIGHT, DUNGEON_WIN_WIDTH, 1, 0}, // DUNGEON

                {MONSTER_WIN_HEIGHT, MONSTER_WIN_WIDTH, // MONSTER LIST
                 CENTER_WINDOWS_Y(DUNGEON_WIN_HEIGHT, MONSTER_WIN_HEIGHT),
                 CENTER_WINDOWS_X(DUNGEON_WIN_WIDTH, MONSTER_WIN_WIDTH)},

                {EQUIPMENT_WIN_HEIGHT, EQUIPMENT_WIN_WIDTH, // EQUIPMENT
                 CENTER_WINDOWS_Y(DUNGEON_WIN_HEIGHT, EQUIPMENT_WIN_HEIGHT),
                 CENTER_WINDOWS_X(DUNGEON_WIN_WIDTH, EQUIPMENT_WIN_WIDTH)},

                {INVENTORY_WIN_HEIGHT, INVENTORY_WIN_WIDTH, // INVENTORY
                 CENTER_WINDOWS_Y(DUNGEON_WIN_HEIGHT, INVENTORY_WIN_HEIGHT),
                 CENTER_WINDOWS_X(DUNGEON_WIN_WIDTH, INVENTORY_WIN_WIDTH)},

                {LORE_WIN_HEIGHT, LORE_WIN_WIDTH, // LORE
                 CENTER_WINDOWS_Y(DUNGEON_WIN_HEIGHT, LORE_WIN_HEIGHT),
                 CENTER_WINDOWS_X(DUNGEON_WIN_WIDTH, LORE_WIN_WIDTH)},

                {STATUS_HEIGHT, DUNGEON_WIN_WIDTH, MESSAGE_HEIGHT + DUNGEON_WIN_HEIGHT, 0} // STATUS
        };

#undef CENTER_WINDOWS_Y
#undef CENTER_WINDOWS_X

        enum class UIMode
        {
            DUNGEON,
            MONSTER_LIST,
            INVENTORY,
            EQUIPMENT,
            TELEPORT,
            LORE_MENU,
            TARGETING
        };

        void check_for_terminal_resize();
        void display_message(const char *format, ...);
        void display_status(const char *format, ...);
        void display_title();
        void update_game_window();

        bool wait_for_input(int &dx, int &dy, bool &force);
        bool get_player_input(int &dx, int &dy, bool &force);

        bool running = true;
        void stop() { running = false; }

        WINDOW *win(WindowID id) { return windows[static_cast<size_t>(id)]; }

    private:
        GameContext &game;
        void init_curses();

        std::array<WINDOW *, static_cast<size_t>(WindowID::COUNT)> windows{};

        // for terminal resize handling
        uint16_t term_x = 0;
        uint16_t term_y = 0;

        // display functions per mode
        void display_dungeon();
        void display_monster_list();
        void display_inventory_list();
        void display_equipment_list();
        void display_monster_lore_menu();
        void display_item_lore_menu();
        void display_cursor_select(); // teleport + search

        // getting inputs per mode, returns true if input should advance game state
        bool handle_dungeon_input(Command cmd, int &dx, int &dy, bool &force);
        bool handle_monster_list_input(Command cmd, int &dx, int &dy, bool &force);
        bool handle_inventory_input(Command cmd, int &dx, int &dy, bool &force);
        bool handle_equipment_input(Command cmd, int &dx, int &dy, bool &force);
        bool handle_lore_input(Command cmd, int &dx, int &dy, bool &force);          // monster + item
        bool handle_cursor_select_input(Command cmd, int &dx, int &dy, bool &force); // teleport + search

        // UI State
        UIMode mode = UIMode::DUNGEON;
        mapsize_t cursor_x = 0;
        mapsize_t cursor_y = 0;
        bool fog_of_war = true;
        bool show_hardness = false;

        int vert_scroll = 0;
        size_t list_open_frame = 0;

        Command selecting_slot_cmd = Command::NONE;

        Object *viewed_item = nullptr;
        Monster *viewed_monster = nullptr;

        std::size_t frame = 0;
        timeval fps_timeout = {0, 0};
        float target_fps;
    };

} // namespace ui

// Helper functions
static inline int mvscrollnstr(WINDOW *win, int y, int x, int n, int scroll, const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::string str(buffer);

    if ((size_t)n >= str.size())
        return mvwaddnstr(win, y, x, str.c_str(), n); // No need to scroll at all

    // Compute real scroll to guarantee the last character is shown
    int max_scroll = std::max(0, (int)str.size() - n);
    int offset = std::min(scroll, max_scroll);

    std::string view = str.substr(offset, n);
    return mvwaddnstr(win, y, x, view.c_str(), n);
}