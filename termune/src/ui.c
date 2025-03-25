#include "ui.h"
#include <stdlib.h>
#include <string.h>

#define STATUS_HEIGHT 1
#define MONSTER_WIN_WIDTH 30
#define MONSTER_WIN_HEIGHT 10
#define DEBUG_WIDTH 5

ui_context *ui_init()
{
    ui_context *ctx = (ui_context *)malloc(sizeof(*ctx));

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int total_width = DUNGEON_WIDTH + DEBUG_WIDTH;

    ctx->dungeon_win = newwin(DUNGEON_HEIGHT, DUNGEON_WIDTH, 0, 0);
    ctx->status_win = newwin(STATUS_HEIGHT, total_width, DUNGEON_HEIGHT, 0);
    ctx->monster_win = NULL;

    ctx->show_monster_win = 0;
    ctx->monster_win_scroll = 0;

    refresh();

    return ctx;
}

void ui_shutdown(ui_context *ctx)
{
    if (ctx->dungeon_win)
        delwin(ctx->dungeon_win);
    if (ctx->status_win)
        delwin(ctx->status_win);
    if (ctx->monster_win)
        delwin(ctx->monster_win);
    endwin();

    free(ctx);
}

void ui_display_title(ui_context *ctx)
{
    static const char *title_art[] = {
        "@@@@@@@  @@@@@@@@  @@@@@@@   @@@@@@@@@@   @@@  @@@  @@@  @@@  @@@@@@@@",
        "@@@@@@@  @@@@@@@@  @@@@@@@@  @@@@@@@@@@@  @@@  @@@  @@@@ @@@  @@@@@@@@",
        "  @@!    @@!       @@!  @@@  @@! @@! @@!  @@!  @@@  @@!@!@@@  @@!     ",
        "  !@!    !@!       !@!  @!@  !@! !@! !@!  !@!  @!@  !@!!@!@!  !@!     ",
        "  @!!    @!!!:!    @!@!!@!   @!! !!@ @!@  @!@  !@!  @!@ !!@!  @!!!:!  ",
        "  !!!    !!!!!:    !!@!@!    !@!   ! !@!  !@!  !!!  !@!  !!!  !!!!!:  ",
        "  !!:    !!:       !!: :!!   !!:     !!:  !!:  !!!  !!:  !!!  !!:     ",
        "  :!:    :!:       :!:  !:!  :!:     :!:  :!:  !:!  :!:  !:!  :!:     ",
        "   ::     :: ::::  ::   :::  :::     ::   ::::: ::   ::   ::   :: ::::",
        "   :     : :: ::    :   : :   :      :     : :  :   ::    :   : :: :: ",
    };

    werase(ctx->dungeon_win);

    for (int y = 0; y < 10; y++)
    {
        mvwprintw(ctx->dungeon_win, 3 + y, 5, "%s", title_art[y]);
    }

    mvwprintw(ctx->dungeon_win, 16, 48, "a rougelike by connor tynan");

    box(ctx->dungeon_win, 0, 0);
    wrefresh(ctx->dungeon_win);

    UI_STATUS(ctx, "Press any key to start...");
}

void ui_display_dungeon(ui_context *ctx, const game_context *g)
{
    werase(ctx->dungeon_win);

    char display[DUNGEON_WIDTH * DUNGEON_HEIGHT];
    game_generate_display_buffer(g, display);

    for (int y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (int x = 0; x < DUNGEON_WIDTH; x++)
        {
            mvwaddch(ctx->dungeon_win, y, x, display[x + y * DUNGEON_WIDTH]);
        }
    }

    box(ctx->dungeon_win, 0, 0);
    wrefresh(ctx->dungeon_win);

    UI_STATUS(ctx, "HP: %d  Monsters left: %ld", g->player.alive, g->alive_monsters);
}

void ui_display_monster_list(ui_context *ctx, const game_context *g)
{
    if (!ctx->monster_win)
    {
        int startx = (DUNGEON_WIDTH - MONSTER_WIN_WIDTH) / 2;
        int starty = (DUNGEON_HEIGHT - MONSTER_WIN_HEIGHT) / 2;
        ctx->monster_win = newwin(MONSTER_WIN_HEIGHT, MONSTER_WIN_WIDTH, starty, startx);
    }

    werase(ctx->monster_win);
    box(ctx->monster_win, 0, 0);
    mvwprintw(ctx->monster_win, 0, 2, " Monster List ");

    int row = 1;
    int skipped = 0;
    int visible = MONSTER_WIN_HEIGHT - 2;

    for (int i = 0; i < g->num_monsters && row <= visible; i++)
    {
        const monster *m = &g->monsters[i];
        if (!m->alive)
            continue;

        if (skipped++ < ctx->monster_win_scroll)
            continue;

        int rel_x = m->x - g->player.x;
        int rel_y = m->y - g->player.y;

        mvwprintw(ctx->monster_win, row++, 1, "%c at %d %s, %d %s",
                  "0123456789abcdef"[m->characteristics.flags],
                  abs(rel_y), rel_y < 0 ? "north" : "south",
                  abs(rel_x), rel_x < 0 ? "west" : "east");
    }

    wrefresh(ctx->monster_win);
}

ui_command ui_get_player_input()
{
    int ch = getch();

    switch (ch)
    {
    case '7':
    case 'y':
        return CMD_MOVE_NW;
    case '8':
    case 'k':
        return CMD_MOVE_N;
    case '9':
    case 'u':
        return CMD_MOVE_NE;
    case '6':
    case 'l':
        return CMD_MOVE_E;
    case '3':
    case 'n':
        return CMD_MOVE_SE;
    case '2':
    case 'j':
        return CMD_MOVE_S;
    case '1':
    case 'b':
        return CMD_MOVE_SW;
    case '4':
    case 'h':
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
    case KEY_UP:
        return CMD_MONSTER_LIST_SCROLL_UP;
    case KEY_DOWN:
        return CMD_MONSTER_LIST_SCROLL_DOWN;
    case 27:
        return CMD_EXIT_MONSTER_LIST;
    case 'Q':
        return CMD_QUIT;
    default:
        return CMD_NONE;
    }
}

void ui_handle_player_input(ui_context *ctx, game_context *g)
{
    ui_command cmd = ui_get_player_input();

    if (ctx->show_monster_win)
    {
        int visible_rows = MONSTER_WIN_HEIGHT - 2;
        int scroll_max = g->alive_monsters > visible_rows
                             ? g->alive_monsters - visible_rows
                             : 0;

        switch (cmd)
        {
        case CMD_MONSTER_LIST_SCROLL_DOWN:
            if (ctx->monster_win_scroll < scroll_max)
                ctx->monster_win_scroll++;
            return;

        case CMD_MONSTER_LIST_SCROLL_UP:
            if (ctx->monster_win_scroll > 0)
                ctx->monster_win_scroll--;
            return;

        case CMD_EXIT_MONSTER_LIST:
            ctx->show_monster_win = 0;
            ctx->monster_win_scroll = 0;
            return;
        case CMD_QUIT:
            g->running = 0;
            return;
        default:
            return;
        }
    }
    else
    {
        switch (cmd)
        {
        case CMD_MOVE_NW:
            player_move(g, -1, -1);
            break;
        case CMD_MOVE_N:
            player_move(g, 0, -1);
            break;
        case CMD_MOVE_NE:
            player_move(g, 1, -1);
            break;
        case CMD_MOVE_E:
            player_move(g, 1, 0);
            break;
        case CMD_MOVE_SE:
            player_move(g, 1, 1);
            break;
        case CMD_MOVE_S:
            player_move(g, 0, 1);
            break;
        case CMD_MOVE_SW:
            player_move(g, -1, 1);
            break;
        case CMD_MOVE_W:
            player_move(g, -1, 0);
            break;
        case CMD_STAIRS_UP:
            break;
        case CMD_STAIRS_DOWN:
            break;
        case CMD_MONSTER_LIST:
            ctx->show_monster_win = 1;
            return;
        case CMD_QUIT:
            g->running = 0;
            return;
        case CMD_REST:
        default:
            return;
        }
    }

    game_process_events(g);
    g->running = g->alive_monsters > 0 && g->player.alive;
}

void ui_display_game(ui_context *ctx, game_context *g)
{
    if (ctx->show_monster_win)
    {
        ui_display_monster_list(ctx, g);
    }
    else
    {
        ui_display_dungeon(ctx, g);
    }
}
