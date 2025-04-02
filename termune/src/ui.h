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
#include "types.h"
#include "game_context.h"

class UiManager
{
public:
    explicit UiManager(GameContext &game);
    ~UiManager();

    static constexpr int MESSAGE_HEIGHT = 1;
    static constexpr int STATUS_HEIGHT = 2;
    static constexpr int MONSTER_WIN_WIDTH = 30;
    static constexpr int MONSTER_WIN_HEIGHT = 10;

    enum class Command
    {
        NONE,
        MOVE_NW,
        MOVE_N,
        MOVE_NE,
        MOVE_E,
        MOVE_SE,
        MOVE_S,
        MOVE_SW,
        MOVE_W,
        REST,
        STAIRS_UP,
        STAIRS_DOWN,
        MONSTER_LIST,
        MONSTER_LIST_SCROLL_UP,
        MONSTER_LIST_SCROLL_DOWN,
        EXIT_MONSTER_LIST,
        TOGGLE_FOG_OF_WAR,
        TOGGLE_TELEPORT_MODE,
        RANDOM_TELEPORT,
        QUIT
    };

    enum class WindowID
    {
        MESSAGE,
        DUNGEON,
        MONSTER,
        STATUS
    };

    void check_for_terminal_resize();
    void display_message(const char *format, ...);
    void display_status(const char *format, ...);
    void display_title();
    void update_game_window();
    void handle_player_input();
    void stop() { running = false; }

    bool running = true;

    WINDOW *operator[](WindowID id) { return win(id); }
    WINDOW *win(WindowID id) { return windows[static_cast<size_t>(id)]; }

private:
    GameContext &game;

    void display_dungeon();
    void display_monster_list();
    void display_teleport_menu();
    Command get_command_from_key(int ch);

    std::array<WINDOW *, 4> windows{}; // MESSAGE, DUNGEON, MONSTER, STATUS

    uint16_t term_x = 0;
    uint16_t term_y = 0;

    bool show_monster_window = false;
    entity_id_t monster_scroll = 0;

    mapsize_t teleport_cursor_x = 0;
    mapsize_t teleport_cursor_y = 0;
};
