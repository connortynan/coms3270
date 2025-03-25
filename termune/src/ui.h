#pragma once

#include "game_context.h"
#include <ncurses.h>

#define UI_STATUS(ctx, ...)                              \
    do                                                   \
    {                                                    \
        werase((ctx)->status_win);                       \
        mvwprintw((ctx)->status_win, 0, 0, __VA_ARGS__); \
        wrefresh((ctx)->status_win);                     \
    } while (0)

typedef struct
{
    WINDOW *dungeon_win;
    WINDOW *status_win;
    WINDOW *monster_win;

    uint8_t show_monster_win;
    uint64_t monster_win_scroll;
} ui_context;

typedef enum
{
    CMD_NONE,
    CMD_MOVE_NW,
    CMD_MOVE_N,
    CMD_MOVE_NE,
    CMD_MOVE_E,
    CMD_MOVE_SE,
    CMD_MOVE_S,
    CMD_MOVE_SW,
    CMD_MOVE_W,
    CMD_REST,
    CMD_STAIRS_UP,
    CMD_STAIRS_DOWN,
    CMD_MONSTER_LIST,
    CMD_MONSTER_LIST_SCROLL_UP,
    CMD_MONSTER_LIST_SCROLL_DOWN,
    CMD_EXIT_MONSTER_LIST,
    CMD_QUIT
} ui_command;

ui_context *ui_init();
void ui_shutdown(ui_context *ctx);

void ui_handle_player_input(ui_context *ctx, game_context *g);
void ui_display_game(ui_context *ctx, game_context *g);

void ui_display_title(ui_context *ctx);
