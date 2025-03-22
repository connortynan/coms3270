#pragma once

#include "game_context.h"

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
    CMD_QUIT
} ui_command;

void ui_init();
void ui_shutdown();

void ui_draw_dungeon(const game_context *g);
ui_command ui_get_player_input();
void ui_display_message(const char *msg);
void ui_display_monster_list(const game_context *g);
