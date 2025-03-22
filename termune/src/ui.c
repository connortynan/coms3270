#include "ui.h"
#include <ncurses.h>

void ui_init()
{
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

void ui_shutdown()
{
    endwin();
}

void ui_draw_dungeon(const game_context *g)
{
    clear();

    char display[DUNGEON_WIDTH * DUNGEON_HEIGHT];
    game_generate_display_buffer(g, display);

    for (int y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (int x = 0; x < DUNGEON_WIDTH; x++)
        {
            mvaddch(y + 1, x, display[x + y * DUNGEON_WIDTH]);
        }
    }

    mvprintw(0, 0, "HP: %d  Monsters left: %d", g->player.alive, g->alive_monsters);
    refresh();
}

ui_command ui_get_player_input()
{
    int ch = getch();

    switch (ch)
    {
    case '7':
    case 'y':
    case KEY_HOME:
        return CMD_MOVE_NW;
    case '8':
    case 'k':
    case KEY_UP:
        return CMD_MOVE_N;
    case '9':
    case 'u':
    case KEY_PPAGE:
        return CMD_MOVE_NE;
    case '6':
    case 'l':
    case KEY_RIGHT:
        return CMD_MOVE_E;
    case '3':
    case 'n':
    case KEY_NPAGE:
        return CMD_MOVE_SE;
    case '2':
    case 'j':
    case KEY_DOWN:
        return CMD_MOVE_S;
    case '1':
    case 'b':
    case KEY_END:
        return CMD_MOVE_SW;
    case '4':
    case 'h':
    case KEY_LEFT:
        return CMD_MOVE_W;
    case '>':
        return CMD_STAIRS_DOWN;
    case '<':
        return CMD_STAIRS_UP;
    case '5':
    case ' ':
    case '.':
        return CMD_REST;
    case 'm':
        return CMD_MONSTER_LIST;
    case 'Q':
        return CMD_QUIT;
    default:
        return CMD_NONE;
    }
}

void ui_display_message(const char *msg)
{
    mvprintw(22, 0, "%s", msg);
    clrtoeol();
    refresh();
}

void ui_display_monster_list(const game_context *g)
{
    clear();
    mvprintw(0, 0, "Monster List (ESC to return):");

    int row = 1;

    for (int i = 0; i < g->num_monsters; i++)
    {
        const monster *m = &g->monsters[i];
        if (!m->alive)
            continue;

        int rel_x = m->x - g->player.x;
        int rel_y = m->y - g->player.y;
        mvprintw(row++, 0, "%c at %d %s, %d %s",
                 "0123456789abcdef"[m->characteristics.flags],
                 abs(rel_y),
                 rel_y < 0 ? "north" : "south",
                 abs(rel_x),
                 rel_x < 0 ? "west" : "east");
    }

    refresh();

    int ch;
    do
    {
        ch = getch();
    } while (ch != 27); // ESC key to exit monster list
}
