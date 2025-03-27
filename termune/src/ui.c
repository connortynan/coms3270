#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#include "dungeon.h"

#define MESSAGE_HEIGHT 1
#define STATUS_HEIGHT 2
#define MONSTER_WIN_WIDTH 30
#define MONSTER_WIN_HEIGHT 10

#define COLOR_LGRAY 126
#define COLOR_DGRAY 127

#define UI_ATTEMPT_PLAYER_MOVE(g, ctx, dx, dy)           \
    if (!player_move((g), (dx), (dy)))                   \
    {                                                    \
        UI_MESSAGE((ctx), "There's a wall in the way!"); \
        return;                                          \
    }                                                    \
    break;

void ui_resize_windows(ui_context *ctx, int term_rows, int term_cols);

ui_context *ui_init()
{
    ui_context *ctx = (ui_context *)malloc(sizeof(*ctx));

    setlocale(LC_ALL, "");
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    set_escdelay(0);

    // set color pairs for title
    start_color();
    use_default_colors();

    init_color(COLOR_LGRAY, 500, 500, 500);
    init_color(COLOR_DGRAY, 250, 250, 250);

    init_pair(1, COLOR_WHITE, COLOR_LGRAY); // Top layer
    init_pair(2, COLOR_LGRAY, COLOR_DGRAY); // Middle layer
    init_pair(3, COLOR_DGRAY, -1);          // Bottom layer

    ctx->message_win = newwin(MESSAGE_HEIGHT, DUNGEON_WIDTH, 0, 0);
    ctx->dungeon_win = newwin(DUNGEON_HEIGHT, DUNGEON_WIDTH, MESSAGE_HEIGHT, 0);
    ctx->status_win = newwin(STATUS_HEIGHT, DUNGEON_WIDTH, MESSAGE_HEIGHT + DUNGEON_HEIGHT, 0);
    ctx->monster_win = newwin(MONSTER_WIN_HEIGHT, MONSTER_WIN_WIDTH,
                              (DUNGEON_HEIGHT - MONSTER_WIN_HEIGHT) / 2 + MESSAGE_HEIGHT,
                              (DUNGEON_WIDTH - MONSTER_WIN_WIDTH) / 2);

    ctx->show_monster_win = 0;
    ctx->monster_win_scroll = 0;
    ctx->running = 1;

    ui_resize_windows(ctx, getmaxy(stdscr), getmaxx(stdscr));
    refresh();

    return ctx;
}

void ui_shutdown(ui_context *ctx)
{
    if (ctx->message_win)
        delwin(ctx->message_win);
    if (ctx->dungeon_win)
        delwin(ctx->dungeon_win);
    if (ctx->monster_win)
        delwin(ctx->monster_win);
    if (ctx->status_win)
        delwin(ctx->status_win);
    endwin();

    free(ctx);
}

void ui_resize_windows(ui_context *ctx, int term_rows, int term_cols)
{
    ctx->term_y = term_rows;
    ctx->term_x = term_cols;

    int total_height = MESSAGE_HEIGHT + DUNGEON_HEIGHT + STATUS_HEIGHT;

    int start_y = (term_rows - total_height > 0) ? (term_rows - total_height) / 2 : 0;
    int start_x = (term_cols - DUNGEON_WIDTH > 0) ? (term_cols - DUNGEON_WIDTH) / 2 : 0;

    // Calculate new positions
    int msg_y = start_y;
    int msg_x = start_x;
    int dung_y = start_y + MESSAGE_HEIGHT;
    int dung_x = start_x;
    int stat_y = dung_y + DUNGEON_HEIGHT;
    int stat_x = start_x;
    int mon_y = dung_y + (DUNGEON_HEIGHT - MONSTER_WIN_HEIGHT) / 2;
    int mon_x = dung_x + (DUNGEON_WIDTH - MONSTER_WIN_WIDTH) / 2;

    // Move existing windows
    mvwin(ctx->message_win, msg_y, msg_x);
    mvwin(ctx->dungeon_win, dung_y, dung_x);
    mvwin(ctx->status_win, stat_y, stat_x);
    mvwin(ctx->monster_win, mon_y, mon_x);

    // Force resize of windows to avoid clipping
    wresize(ctx->message_win, MESSAGE_HEIGHT, DUNGEON_WIDTH);
    wresize(ctx->dungeon_win, DUNGEON_HEIGHT, DUNGEON_WIDTH);
    wresize(ctx->status_win, STATUS_HEIGHT, DUNGEON_WIDTH);
    wresize(ctx->monster_win, MONSTER_WIN_HEIGHT, MONSTER_WIN_WIDTH);
}

int ui_check_resize(ui_context *ctx)
{
    int new_y, new_x;
    getmaxyx(stdscr, new_y, new_x);

    if (ctx->term_x != new_x || ctx->term_y != new_y)
    {
        ui_resize_windows(ctx, new_y, new_x);
        clear();
        refresh();
        return 1;
    }

    return 0;
}

void ui_display_title(ui_context *ctx)
{
    static const wchar_t *title_art[3][10] = {
#include "title.inc"
    };

    werase(ctx->dungeon_win);

    mvwprintw(ctx->dungeon_win, 16, 48, "a rougelike by connor tynan");

    box(ctx->dungeon_win, 0, 0);

    for (int i = 2; i >= 0; i--)
    {
        wattron(ctx->dungeon_win, COLOR_PAIR(i + 1));
        for (int y = 0; y < 10; y++)
        {
            const wchar_t *line = title_art[i][y];

            for (int x = 0; line[x] != L'\0'; x++)
            {
                if (line[x] != L' ')
                    mvwaddnwstr(ctx->dungeon_win, 3 + y, x, &line[x], 1);
            }
        }
        wattroff(ctx->dungeon_win, COLOR_PAIR(i + 1));
    }

    wrefresh(ctx->dungeon_win);
    UI_MESSAGE(ctx, "Press any key to start...");
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

    if (ui_check_resize(ctx))
        return;

    // clear message
    UI_MESSAGE(ctx, " ");

    // break runs event queue,
    // return skips event queue
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
            ctx->running = 0;
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
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, -1, -1);
        case CMD_MOVE_N:
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, 0, -1);
        case CMD_MOVE_NE:
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, 1, -1);
        case CMD_MOVE_E:
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, 1, 0);
        case CMD_MOVE_SE:
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, 1, 1);
        case CMD_MOVE_S:
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, 0, 1);
        case CMD_MOVE_SW:
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, -1, 1);
        case CMD_MOVE_W:
            UI_ATTEMPT_PLAYER_MOVE(g, ctx, -1, 0);
        case CMD_STAIRS_UP:
            if (g->current_dungeon->cell_types[g->player.y][g->player.x] == CELL_STAIR_UP)
            {
                game_regenerate_dungeon(g);
                break;
            }
            return;
        case CMD_STAIRS_DOWN:
            if (g->current_dungeon->cell_types[g->player.y][g->player.x] == CELL_STAIR_DOWN)
            {
                game_regenerate_dungeon(g);
                break;
            }
            return;
        case CMD_REST:
            break;
        case CMD_MONSTER_LIST:
            ctx->show_monster_win = 1;
            return;
        case CMD_QUIT:
            ctx->running = 0;
            return;
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
