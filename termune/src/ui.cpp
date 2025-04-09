#include "ui.hpp"

#include <cstdarg>
#include <cstring>
#include <clocale>
#include <unistd.h>
#include <cmath>
#include <algorithm>
#include "dungeon.hpp"
#include "game_context.hpp"

static constexpr short COLOR_TITLE_LIGHT = 64;
static constexpr short COLOR_TITLE_DARK = 65;

void UiManager::init_curses()
{
    setlocale(LC_ALL, "");
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    set_escdelay(0);
    start_color();
    use_default_colors();

    init_color(COLOR_TITLE_LIGHT, 500, 500, 500);
    init_color(COLOR_TITLE_DARK, 250, 250, 250);
    init_pair(1, COLOR_WHITE, COLOR_TITLE_LIGHT);
    init_pair(2, COLOR_TITLE_LIGHT, COLOR_TITLE_DARK);
    init_pair(3, COLOR_TITLE_DARK, -1);

    for (int i = 0; i < 64; ++i)
    {
        // map 0-63 to grayscale from dark to light (0-1000)
        int shade = static_cast<int>(1000.0 * i / 63);
        int color_id = 128 + i;
        init_color(color_id, shade, shade, shade);
        init_pair(color_id, -1, color_id);
    }
}

UiManager::UiManager(GameContext &game) : game(game)
{
    init_curses();

    windows[static_cast<size_t>(WindowID::MESSAGE)] = newwin(MESSAGE_HEIGHT, game.dungeon.width, 0, 0);
    windows[static_cast<size_t>(WindowID::DUNGEON)] = newwin(game.dungeon.height, game.dungeon.width, MESSAGE_HEIGHT, 0);
    windows[static_cast<size_t>(WindowID::MONSTER)] = newwin(MONSTER_WIN_HEIGHT, MONSTER_WIN_WIDTH, 0, 0);
    windows[static_cast<size_t>(WindowID::STATUS)] = newwin(STATUS_HEIGHT, game.dungeon.width, MESSAGE_HEIGHT + game.dungeon.height, 0);

    check_for_terminal_resize();
    refresh();
}

UiManager::~UiManager()
{
    for (WINDOW *w : windows)
        if (w)
            delwin(w);
    endwin();
}

void UiManager::check_for_terminal_resize()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    if (term_x == cols && term_y == rows)
        return;

    term_x = cols;
    term_y = rows;

    int total_height = MESSAGE_HEIGHT + game.dungeon.height + STATUS_HEIGHT;
    int start_y = std::max(0, (rows - total_height) / 2);
    int start_x = std::max(0, (cols - game.dungeon.width) / 2);

    mvwin(win(WindowID::MESSAGE), start_y, start_x);
    mvwin(win(WindowID::DUNGEON), start_y + MESSAGE_HEIGHT, start_x);
    mvwin(win(WindowID::STATUS), start_y + MESSAGE_HEIGHT + game.dungeon.height, start_x);

    int mon_y = start_y + MESSAGE_HEIGHT + (game.dungeon.height - MONSTER_WIN_HEIGHT) / 2;
    int mon_x = start_x + (game.dungeon.width - MONSTER_WIN_WIDTH) / 2;
    mvwin(win(WindowID::MONSTER), mon_y, mon_x);

    wresize(win(WindowID::MESSAGE), MESSAGE_HEIGHT, game.dungeon.width);
    wresize(win(WindowID::DUNGEON), game.dungeon.height, game.dungeon.width);
    wresize(win(WindowID::STATUS), STATUS_HEIGHT, game.dungeon.width);
    wresize(win(WindowID::MONSTER), MONSTER_WIN_HEIGHT, MONSTER_WIN_WIDTH);

    clear();
    refresh();
}

void UiManager::display_message(const char *format, ...)
{
    werase(win(WindowID::MESSAGE));
    va_list args;
    va_start(args, format);
    vw_printw(win(WindowID::MESSAGE), format, args);
    va_end(args);
    wrefresh(win(WindowID::MESSAGE));
}

void UiManager::display_status(const char *format, ...)
{
    werase(win(WindowID::STATUS));
    va_list args;
    va_start(args, format);
    vw_printw(win(WindowID::STATUS), format, args);
    va_end(args);
    wrefresh(win(WindowID::STATUS));
}

void UiManager::display_title()
{
#ifdef USE_WNCURSES
    static const wchar_t *title_art[3][10] = {
#include "art/title.inc"
    };
#else
    static const char *title_art[3][10] = {
#include "art/title.inc.compat"
    };
#endif

    werase(win(WindowID::DUNGEON));
    box(win(WindowID::DUNGEON), 0, 0);
    mvwprintw(win(WindowID::DUNGEON), 16, 48, "a roguelike by connor tynan");

    for (int i = 2; i >= 0; i--)
    {
        wattron(win(WindowID::DUNGEON), COLOR_PAIR(i + 1));
        for (int y = 0; y < 10; y++)
        {
#ifdef USE_WNCURSES
            const wchar_t *line = title_art[i][y];
            for (int x = 0; line[x] != L'\0'; x++)
                if (line[x] != L' ')
                    mvwaddnwstr(win(WindowID::DUNGEON), 3 + y, x, &line[x], 1);
#else
            const char *line = title_art[i][y];
            for (int x = 0; line[x]; x++)
                if (line[x] != ' ')
                    mvwaddch(win(WindowID::DUNGEON), 3 + y, x, line[x]);
#endif
        }
        wattroff(win(WindowID::DUNGEON), COLOR_PAIR(i + 1));
    }

    wrefresh(win(WindowID::DUNGEON));
    display_message("Press any key to start...");
}

void UiManager::display_dungeon()
{
    WINDOW *w = win(WindowID::DUNGEON);
    werase(w);

    for (mapsize_t y = 0; y < game.dungeon.height; y++)
        for (mapsize_t x = 0; x < game.dungeon.width; x++)
        {
            int color_idx = 128 + std::clamp(game.dungeon.hardness_grid.at(x, y) / 4, 0, 63);
            bool visible = game.visibility_at(x, y).visible || !fog_of_war;
            if (visible)
                wattron(w, A_BOLD);
            if (show_hardness)
                wattron(w, COLOR_PAIR(color_idx));

            Dungeon::cell_type_t displayed_cell = fog_of_war
                                                      ? game.visibility_at(x, y).last_seen
                                                      : game.dungeon.type_grid.at(x, y);

            const auto &entity = game.entity_at(x, y);

            if (visible && entity)
                mvwaddch(w, y, x, entity->ch);
            else
                mvwaddch(w, y, x, Dungeon::char_map(displayed_cell));

            if (visible)
                wattroff(w, A_BOLD);
            if (show_hardness)
                wattroff(w, COLOR_PAIR(color_idx));
        }

    box(win(WindowID::DUNGEON), 0, 0);
    wrefresh(win(WindowID::DUNGEON));
    display_status("HP: %d  Monsters left: %ld, Tick: %ld", game.player->alive, game.alive_monsters.size(), game.current_tick());
}

void UiManager::display_monster_list()
{
    WINDOW *w = win(WindowID::MONSTER);
    werase(w);
    box(w, 0, 0);
    mvwprintw(w, 0, 2, " Monster List ");

    int row = 1;
    int skipped = 0;
    for (auto &m : game.alive_monsters)
    {
        if (skipped++ < monster_scroll)
            continue;
        if (!m->alive)
            continue;
        if (row > MONSTER_WIN_HEIGHT - 2)
            break;

        int rel_x = m->x - game.player->x;
        int rel_y = m->y - game.player->y;
        mvwprintw(w, row++, 1, "%c at %d %s, %d %s",
                  m->ch,
                  std::abs(rel_y), rel_y < 0 ? "north" : "south",
                  std::abs(rel_x), rel_x < 0 ? "west" : "east");
    }

    wrefresh(w);
}

void UiManager::display_teleport_menu()
{
    werase(win(WindowID::DUNGEON));

    for (mapsize_t y = 0; y < game.dungeon.height; ++y)
        for (mapsize_t x = 0; x < game.dungeon.width; ++x)
        {
            char ch;
            if (x == teleport_cursor_x && y == teleport_cursor_y)
                ch = '*';
            else
            {
                const auto &entity = game.entity_at(x, y);
                if (entity)
                    ch = entity->ch;
                else
                    ch = Dungeon::char_map(game.dungeon.type_grid.at(x, y));
            }
            mvwaddch(win(WindowID::DUNGEON), y, x, ch);
        }

    box(win(WindowID::DUNGEON), 0, 0);
    wrefresh(win(WindowID::DUNGEON));
    display_status("Teleport mode: move cursor, 'g' to teleport, 'r' random");
}

void UiManager::update_game_window()
{
    if (teleport_mode)
        display_teleport_menu();
    else if (show_monster_window)
        display_monster_list();
    else
        display_dungeon();
}

UiManager::Command UiManager::get_command_from_key(int ch)
{
    switch (ch)
    {
    case '7':
    case 'y':
        return Command::MOVE_NW;
    case '8':
    case 'k':
        return Command::MOVE_N;
    case '9':
    case 'u':
        return Command::MOVE_NE;
    case '6':
    case 'l':
        return Command::MOVE_E;
    case '3':
    case 'n':
        return Command::MOVE_SE;
    case '2':
    case 'j':
        return Command::MOVE_S;
    case '1':
    case 'b':
        return Command::MOVE_SW;
    case '4':
    case 'h':
        return Command::MOVE_W;
    case '>':
        return Command::STAIRS_DOWN;
    case '<':
        return Command::STAIRS_UP;
    case '5':
    case ' ':
    case '.':
        return Command::REST;
    case 'm':
        return Command::MONSTER_LIST;
    case KEY_UP:
        return Command::MONSTER_LIST_SCROLL_UP;
    case KEY_DOWN:
        return Command::MONSTER_LIST_SCROLL_DOWN;
    case 27:
        return Command::EXIT_MONSTER_LIST;
    case 'f':
        return Command::TOGGLE_FOG_OF_WAR;
    case 'Q':
        return Command::QUIT;
    case 'g':
        return Command::TOGGLE_TELEPORT_MODE;
    case 'r':
        return Command::RANDOM_TELEPORT;
    case 'H':
        return Command::TOGGLE_SHOW_HARDNESS;
    default:
        return Command::NONE;
    }
}

void UiManager::handle_player_input()
{
    Command cmd = get_command_from_key(getch());
    check_for_terminal_resize();
    display_message(" ");

    // todo: bad name!
    bool update_but_dont_move_monsters = false;
    int dx = 0, dy = 0;
    bool move_attempt = true;

    if (teleport_mode)
    {
        switch (cmd)
        {
        case Command::MOVE_NW:
            teleport_cursor_x--;
            teleport_cursor_y--;
            break;
        case Command::MOVE_N:
            teleport_cursor_y--;
            break;
        case Command::MOVE_NE:
            teleport_cursor_x++;
            teleport_cursor_y--;
            break;
        case Command::MOVE_E:
            teleport_cursor_x++;
            break;
        case Command::MOVE_SE:
            teleport_cursor_x++;
            teleport_cursor_y++;
            break;
        case Command::MOVE_S:
            teleport_cursor_y++;
            break;
        case Command::MOVE_SW:
            teleport_cursor_x--;
            teleport_cursor_y++;
            break;
        case Command::MOVE_W:
            teleport_cursor_x--;
            break;
        case Command::TOGGLE_TELEPORT_MODE:
            if (game.dungeon.hardness_grid.at(teleport_cursor_x, teleport_cursor_y) < 255)
            {
                game.player->move(int(teleport_cursor_x) - int(game.player->x),
                                  int(teleport_cursor_y) - int(game.player->y),
                                  game, true);
                teleport_mode = false;
            }
            else
            {
                display_message("Cannot teleport to this location...");
                return;
            }
            update_but_dont_move_monsters = true;
            goto run_update;
        case Command::RANDOM_TELEPORT:
            do
            {
                teleport_cursor_x = rand() % (game.dungeon.width - 2) + 1;
                teleport_cursor_y = rand() % (game.dungeon.height - 2) + 1;
            } while (game.dungeon.hardness_grid.at(teleport_cursor_x, teleport_cursor_y) >= 255);
            game.player->move(int(teleport_cursor_x) - int(game.player->x),
                              int(teleport_cursor_y) - int(game.player->y),
                              game, true);
            teleport_mode = false;
            update_but_dont_move_monsters = true;
            goto run_update;
        case Command::QUIT:
            running = false;
            return;
        default:
            return;
        }
        // Clamp to dungeon bounds
        teleport_cursor_x = std::clamp(teleport_cursor_x, mapsize_t(1), mapsize_t(game.dungeon.width - 2));
        teleport_cursor_y = std::clamp(teleport_cursor_y, mapsize_t(1), mapsize_t(game.dungeon.height - 2));
        return;
    }

    else if (show_monster_window)
    {
        int visible_rows = MONSTER_WIN_HEIGHT - 2;
        int scroll_max = std::max(0UL, game.alive_monsters.size() - visible_rows);

        switch (cmd)
        {
        case Command::MONSTER_LIST_SCROLL_DOWN:
            if (monster_scroll < scroll_max)
                monster_scroll++;
            return;
        case Command::MONSTER_LIST_SCROLL_UP:
            if (monster_scroll > 0)
                monster_scroll--;
            return;
        case Command::EXIT_MONSTER_LIST:
            show_monster_window = false;
            monster_scroll = 0;
            return;
        case Command::QUIT:
            running = false;
            return;
        default:
            return;
        }
    }

    else // show_dungeon
    {
        switch (cmd)
        {
        case Command::MOVE_NW:
            dx = -1;
            dy = -1;
            break;
        case Command::MOVE_N:
            dy = -1;
            break;
        case Command::MOVE_NE:
            dx = 1;
            dy = -1;
            break;
        case Command::MOVE_E:
            dx = 1;
            break;
        case Command::MOVE_SE:
            dx = 1;
            dy = 1;
            break;
        case Command::MOVE_S:
            dy = 1;
            break;
        case Command::MOVE_SW:
            dx = -1;
            dy = 1;
            break;
        case Command::MOVE_W:
            dx = -1;
            break;
        case Command::STAIRS_UP:
            if (game.dungeon.type_grid.at(game.player->x, game.player->y) == Dungeon::CELL_STAIR_UP)
                game.regenerate_dungeon();
            else
                return;
            move_attempt = false;
            break;
        case Command::STAIRS_DOWN:
            if (game.dungeon.type_grid.at(game.player->x, game.player->y) == Dungeon::CELL_STAIR_DOWN)
                game.regenerate_dungeon();
            else
                return;
            move_attempt = false;
            break;
        case Command::REST:
            break;
        case Command::MONSTER_LIST:
            show_monster_window = true;
            return;
        case Command::TOGGLE_FOG_OF_WAR:
            fog_of_war = !fog_of_war;
            return;
        case Command::TOGGLE_TELEPORT_MODE:
            teleport_cursor_x = game.player->x;
            teleport_cursor_y = game.player->y;
            teleport_mode = true;
            return;
        case Command::TOGGLE_SHOW_HARDNESS:
            show_hardness = !show_hardness;
            return;
        case Command::QUIT:
            running = false;
            return;
        default:
            return;
        }

        if (move_attempt && !game.player->move(dx, dy, game))
        {
            display_message("There's a wall in the way!");
            return;
        }
    }

/* label */ run_update:
    game.update_on_change();
    if (!update_but_dont_move_monsters)
        game.process_events();
}
