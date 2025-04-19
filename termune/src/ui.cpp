#include "ui.hpp"

#include <cstdarg>
#include <cstring>
#include <clocale>
#include <unistd.h>
#include <cmath>
#include <sys/select.h>
#include <chrono>
#include <algorithm>

#include "dungeon.hpp"
#include "game_context.hpp"
#include "monster.hpp"
#include "util/colors.hpp"

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
    init_color_pairs(COLOR_BLACK);

    init_color(COLOR_TITLE_LIGHT, 500, 500, 500);
    init_color(COLOR_TITLE_DARK, 250, 250, 250);
    init_pair(64, COLOR_WHITE, COLOR_TITLE_LIGHT);
    init_pair(65, COLOR_TITLE_LIGHT, COLOR_TITLE_DARK);
    init_pair(66, COLOR_TITLE_DARK, COLOR_BLACK);

    for (int i = 0; i < 64; ++i)
    {
        // map 0-63 to grayscale from dark to light (0-1000)
        int shade = static_cast<int>(1000.0 * i / 63);
        int color_id = 128 + i;
        init_color(color_id, shade, shade, shade);
        init_pair(color_id, -1, color_id);
    }

    bkgd(COLOR_PAIR(COLOR_WHITE));
}

UiManager::UiManager(GameContext &game, float fps) : game(game), target_fps(fps)
{
    long usec = static_cast<long>(1e6 / target_fps);
    fps_timeout.tv_sec = usec / 1000000;
    fps_timeout.tv_usec = usec % 1000000;

    init_curses();

    windows[static_cast<size_t>(WindowID::MESSAGE)] = newwin(MESSAGE_HEIGHT, game.dungeon.width, 0, 0);
    wbkgd(windows[static_cast<size_t>(WindowID::MESSAGE)], COLOR_PAIR(COLOR_WHITE));
    windows[static_cast<size_t>(WindowID::DUNGEON)] = newwin(game.dungeon.height, game.dungeon.width, MESSAGE_HEIGHT, 0);
    wbkgd(windows[static_cast<size_t>(WindowID::DUNGEON)], COLOR_PAIR(COLOR_WHITE));
    windows[static_cast<size_t>(WindowID::MONSTER)] = newwin(MONSTER_WIN_HEIGHT, MONSTER_WIN_WIDTH, 0, 0);
    wbkgd(windows[static_cast<size_t>(WindowID::MONSTER)], COLOR_PAIR(COLOR_WHITE));
    windows[static_cast<size_t>(WindowID::STATUS)] = newwin(STATUS_HEIGHT, game.dungeon.width, MESSAGE_HEIGHT + game.dungeon.height, 0);
    wbkgd(windows[static_cast<size_t>(WindowID::STATUS)], COLOR_PAIR(COLOR_WHITE));
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
        wattron(win(WindowID::DUNGEON), COLOR_PAIR(i + 64));
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
        wattroff(win(WindowID::DUNGEON), COLOR_PAIR(i + 64));
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
            bool visible = game.visibility_at(x, y).visible || !fog_of_war;

            // turn A_BOLD on bby default, and undo for non visible tiles
            wattron(w, A_BOLD);
            if (x == game.player.x && y == game.player.y)
            {
                game.player.render(*this, frame);
                continue;
            }

            const auto &entity = game.top_entity_at(x, y);
            if (visible && entity)
            {
                entity->render(*this, frame);
                continue;
            }

            int color_idx = 128 + std::clamp(game.dungeon.hardness_grid.at(x, y) / 4, 0, 63);

            if (!visible)
                wattroff(w, A_BOLD);

            if (show_hardness)
                wattron(w, COLOR_PAIR(color_idx));
            else
                wattron(win(WindowID::DUNGEON), COLOR_PAIR(COLOR_WHITE));

            Dungeon::cell_type_t displayed_cell =
                fog_of_war
                    ? game.visibility_at(x, y).last_seen
                    : game.dungeon.type_grid.at(x, y);

            mvwaddch(w, y, x, Dungeon::char_map(displayed_cell));

            wattroff(w, A_BOLD);
            if (show_hardness)
                wattroff(w, COLOR_PAIR(color_idx));
            else
                wattroff(win(WindowID::DUNGEON), COLOR_PAIR(COLOR_WHITE));
        }

    box(win(WindowID::DUNGEON), 0, 0);
    wrefresh(win(WindowID::DUNGEON));
    display_status("HP: %d  Monsters left: %ld, Tick: %ld",
                   game.player.health,
                   game.filter<Monster>([](const Monster &m)
                                        { return m.active; })
                       .size(),
                   game.current_tick());
}

void UiManager::display_monster_list()
{
    WINDOW *w = win(WindowID::MONSTER);
    werase(w);
    box(w, 0, 0);
    mvwprintw(w, 0, 2, " Monster List ");

    int max_visible = MONSTER_WIN_WIDTH - 2;
    std::vector<std::string> lines;

    // First: collect lines and determine global max scroll
    size_t global_max_scroll = 0;

    for (const auto &m : game.filtered<Monster>([](const Monster &m)
                                                { return m.active; }))
    {
        int rel_x = m->x - game.player.x;
        int rel_y = m->y - game.player.y;

        std::string line = std::string(m->name()) + " at " +
                           std::to_string(std::abs(rel_y)) + (rel_y < 0 ? " north" : " south") + ", " +
                           std::to_string(std::abs(rel_x)) + (rel_x < 0 ? " west" : " east");

        lines.push_back(line);

        if (line.length() > (size_t)max_visible)
        {
            size_t line_scroll = line.length() - max_visible;
            global_max_scroll = std::max(global_max_scroll, line_scroll);
        }
    }

    // Global scroll offset (syncs all lines)
    size_t scroll_cycle_len = global_max_scroll + 2; // +1 for pause at end
    size_t phase = (2 * (frame - monster_list_open_frame) / (size_t)target_fps) % scroll_cycle_len;
    size_t global_scroll = (phase > global_max_scroll) ? global_max_scroll : phase;

    // Draw lines
    int row = 1;
    for (size_t i = monster_vert_scroll; i < lines.size() && row <= MONSTER_WIN_HEIGHT - 2; ++i)
    {
        const std::string &line = lines[i];

        if (line.length() <= (size_t)max_visible)
        {
            mvwaddnstr(w, row++, 1, line.c_str(), max_visible);
        }
        else
        {
            size_t line_max_scroll = line.length() - max_visible;
            size_t effective_scroll = std::min(global_scroll, line_max_scroll);

            std::string visible = line.substr(effective_scroll, max_visible);
            mvwaddnstr(w, row++, 1, visible.c_str(), max_visible);
        }
    }

    wrefresh(w);
}

void UiManager::display_teleport_menu()
{
    werase(win(WindowID::DUNGEON));

    for (mapsize_t y = 0; y < game.dungeon.height; ++y)
        for (mapsize_t x = 0; x < game.dungeon.width; ++x)
        {
            wattron(win(WindowID::DUNGEON), COLOR_PAIR(COLOR_WHITE));
            if (x == teleport_cursor_x && y == teleport_cursor_y)
                mvwaddch(win(WindowID::DUNGEON), y, x, '*');
            else
            {
                const auto &entity = game.top_entity_at(x, y);
                if (entity)
                    entity->render(*this, frame);
                else
                    mvwaddch(win(WindowID::DUNGEON), y, x, Dungeon::char_map(game.dungeon.type_grid.at(x, y)));
            }
        }

    wattroff(win(WindowID::DUNGEON), COLOR_PAIR(COLOR_WHITE));

    box(win(WindowID::DUNGEON), 0, 0);
    wrefresh(win(WindowID::DUNGEON));
    display_status("Teleport mode: move cursor, 'g' to teleport, 'r' random");
}

void UiManager::update_game_window()
{
    check_for_terminal_resize();
    refresh();

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

bool UiManager::wait_for_input(int &dx, int &dy, bool &force)
{
    dx = dy = 0;
    force = false;

    while (running && game.running)
    {
        update_game_window();

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        // Wait for input or timeout
        timeval timeout = fps_timeout;
        int result = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);
        ++frame;

        if (result == -1)
        {
            // Error in select()
            perror("select");
            break;
        }
        else if (result == 0)
        {
            update_game_window();
            continue;
        }
        else if (FD_ISSET(STDIN_FILENO, &fds))
        {
            // Input available
            if (get_player_input(dx, dy, force))
                return true;
        }
    }

    return false;
}

bool UiManager::get_player_input(int &dx, int &dy, bool &force)
{
    force = false;
    dx = dy = 0;
    check_for_terminal_resize();

    Command cmd = get_command_from_key(getch());

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
        {
            if (game.dungeon.hardness_grid.at(teleport_cursor_x, teleport_cursor_y) < 255)
            {
                dx = int(teleport_cursor_x) - int(game.player.x);
                dy = int(teleport_cursor_y) - int(game.player.y);
                teleport_mode = false;
                force = true;
                return true;
            }
            else
            {
                display_message("Cannot teleport to this location...");
                return false;
            }
        }
        case Command::RANDOM_TELEPORT:
        {
            do
            {
                teleport_cursor_x = rand() % (game.dungeon.width - 2) + 1;
                teleport_cursor_y = rand() % (game.dungeon.height - 2) + 1;
            } while (game.dungeon.hardness_grid.at(teleport_cursor_x, teleport_cursor_y) >= 255);
            dx = int(teleport_cursor_x) - int(game.player.x);
            dy = int(teleport_cursor_y) - int(game.player.y);
            teleport_mode = false;
            force = true;
            return true;
        }
        case Command::QUIT:
            running = false;
            return false;
        default:
            break;
        }

        // Clamp bounds
        teleport_cursor_x = std::clamp(teleport_cursor_x, mapsize_t(1), mapsize_t(game.dungeon.width - 2));
        teleport_cursor_y = std::clamp(teleport_cursor_y, mapsize_t(1), mapsize_t(game.dungeon.height - 2));
        return false;
    }

    if (show_monster_window)
    {
        int visible_rows = MONSTER_WIN_HEIGHT - 2;
        int scroll_max = std::max(0UL, game.filter<Monster>().size() - visible_rows);

        switch (cmd)
        {
        case Command::MONSTER_LIST_SCROLL_DOWN:
            if (monster_vert_scroll < scroll_max)
                monster_vert_scroll++;
            break;
        case Command::MONSTER_LIST_SCROLL_UP:
            if (monster_vert_scroll > 0)
                monster_vert_scroll--;
            break;
        case Command::EXIT_MONSTER_LIST:
            show_monster_window = false;
            monster_vert_scroll = 0;
            break;
        case Command::QUIT:
            running = false;
            return true;
        default:
            break;
        }

        return false;
    }

    // Dungeon view input
    switch (cmd)
    {
    case Command::MOVE_N:
        dy = -1;
        return true;
    case Command::MOVE_S:
        dy = 1;
        return true;
    case Command::MOVE_W:
        dx = -1;
        return true;
    case Command::MOVE_E:
        dx = 1;
        return true;
    case Command::MOVE_NW:
        dx = -1;
        dy = -1;
        return true;
    case Command::MOVE_NE:
        dx = 1;
        dy = -1;
        return true;
    case Command::MOVE_SW:
        dx = -1;
        dy = 1;
        return true;
    case Command::MOVE_SE:
        dx = 1;
        dy = 1;
        return true;
    case Command::REST:
        return true;

    case Command::STAIRS_UP:
        if (game.dungeon.type_grid.at(game.player.x, game.player.y) == Dungeon::CELL_STAIR_UP)
            game.regenerate_dungeon();
        return false;

    case Command::STAIRS_DOWN:
        if (game.dungeon.type_grid.at(game.player.x, game.player.y) == Dungeon::CELL_STAIR_DOWN)
            game.regenerate_dungeon();
        return false;

    case Command::MONSTER_LIST:
        show_monster_window = true;
        monster_vert_scroll = 0;
        monster_list_open_frame = frame;
        return false;

    case Command::TOGGLE_FOG_OF_WAR:
        fog_of_war = !fog_of_war;
        return false;

    case Command::TOGGLE_SHOW_HARDNESS:
        show_hardness = !show_hardness;
        return false;

    case Command::TOGGLE_TELEPORT_MODE:
        teleport_cursor_x = game.player.x;
        teleport_cursor_y = game.player.y;
        teleport_mode = true;
        return false;

    case Command::QUIT:
        running = false;
        return true;

    default:
        return false;
    }
}
