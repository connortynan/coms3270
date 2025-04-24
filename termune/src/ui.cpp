#include "ui.hpp"
#include "ui_command.hpp"

#include <cstdarg>
#include <cstring>
#include <clocale>
#include <unistd.h>
#include <sys/select.h>
#include <chrono>
#include <algorithm>

#include "dungeon.hpp"
#include "monster.hpp"
#include "util/colors.hpp"
#include "entity.hpp"

namespace ui
{

    static constexpr short BACKGROUND_COLOR = COLOR_BLACK;
    static constexpr short COLOR_TITLE_LIGHT = 64;
    static constexpr short COLOR_TITLE_DARK = 65;

    Context::Context(GameContext &game, float fps)
        : game(game), target_fps(fps)
    {
        setlocale(LC_ALL, "");
        initscr();
        raw();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        set_escdelay(0);

        init_color_pairs(BACKGROUND_COLOR);

        init_color(COLOR_TITLE_LIGHT, 500, 500, 500);
        init_color(COLOR_TITLE_DARK, 250, 250, 250);
        init_pair(64, COLOR_WHITE, COLOR_TITLE_LIGHT);
        init_pair(65, COLOR_TITLE_LIGHT, COLOR_TITLE_DARK);
        init_pair(66, COLOR_TITLE_DARK, BACKGROUND_COLOR);

        for (int i = 0; i < 64; ++i)
        {
            int shade = static_cast<int>(1000.0 * i / 63);
            int color_id = 128 + i;
            init_color(color_id, shade, shade, shade);
            init_pair(color_id, -1, color_id);
        }

        bkgd(COLOR_PAIR(COLOR_WHITE));

        long usec = static_cast<long>(1e6 / target_fps);
        fps_timeout.tv_sec = usec / 1000000;
        fps_timeout.tv_usec = usec % 1000000;

        for (size_t i = 0; i < static_cast<size_t>(Context::WindowID::COUNT); ++i)
        {
            windows[i] = newwin(WINDOW_SIZE[i][0], WINDOW_SIZE[i][1], WINDOW_SIZE[i][2], WINDOW_SIZE[i][3]);
            wbkgd(windows[i], COLOR_PAIR(COLOR_WHITE));
        }

        check_for_terminal_resize();
        refresh();
    }

    Context::~Context()
    {
        for (WINDOW *w : windows)
            if (w)
                delwin(w);
        endwin();
    }

    bool Context::wait_for_input(int &dx, int &dy, bool &force)
    {
        dx = dy = 0;
        force = false;

        while (running && game.running)
        {
            update_game_window();

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);

            timeval timeout = fps_timeout;
            int result = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);
            ++frame;

            if (result == -1)
            {
                perror("select");
                break;
            }
            else if (result == 0)
            {
                continue;
            }
            else if (FD_ISSET(STDIN_FILENO, &fds))
            {
                if (get_player_input(dx, dy, force))
                    return true;
            }
        }

        return false;
    }

    void Context::update_game_window()
    {
        check_for_terminal_resize();

        switch (mode)
        {
        case UIMode::DUNGEON:
            display_dungeon();
            break;

        case UIMode::MONSTER_LIST:
            display_monster_list();
            break;

        case UIMode::INVENTORY:
            display_inventory_list();
            break;

        case UIMode::EQUIPMENT:
            display_equipment_list();
            break;

        case UIMode::LORE_MENU:
            if (viewed_monster)
                display_monster_lore_menu();
            else if (viewed_item)
                display_item_lore_menu();
            break;

        case UIMode::TELEPORT:
        case UIMode::TARGETING:
            display_cursor_select();
            break;
        }

        refresh();
    }

    bool Context::get_player_input(int &dx, int &dy, bool &force)
    {
        force = false;
        dx = dy = 0;
        check_for_terminal_resize();

        Command cmd = get_command_from_key(getch(), selecting_slot_cmd != Command::NONE);

        switch (mode)
        {
        case UIMode::DUNGEON:
            return handle_dungeon_input(cmd, dx, dy, force);

        case UIMode::MONSTER_LIST:
            return handle_monster_list_input(cmd, dx, dy, force);

        case UIMode::INVENTORY:
            return handle_inventory_input(cmd, dx, dy, force);

        case UIMode::EQUIPMENT:
            return handle_equipment_input(cmd, dx, dy, force);

        case UIMode::TELEPORT:
            return handle_cursor_select_input(cmd, dx, dy, force);

        case UIMode::LORE_MENU:
            return handle_lore_input(cmd, dx, dy, force);

        case UIMode::TARGETING:
            return handle_cursor_select_input(cmd, dx, dy, force);

        default:
            return false;
        }
    }

    void Context::check_for_terminal_resize()
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

        for (size_t i = 0; i < static_cast<size_t>(WindowID::COUNT); ++i)
        {
            int h = WINDOW_SIZE[i][0];
            int w = WINDOW_SIZE[i][1];
            int y = WINDOW_SIZE[i][2];
            int x = WINDOW_SIZE[i][3];

            mvwin(windows[i], start_y + y, start_x + x);
            wresize(windows[i], h, w);
        }

        clear();
        refresh();
    }

    void Context::display_message(const char *format, ...)
    {
        werase(win(WindowID::MESSAGE));
        va_list args;
        va_start(args, format);
        vw_printw(win(WindowID::MESSAGE), format, args);
        va_end(args);
        wrefresh(win(WindowID::MESSAGE));
    }

    void Context::display_status(const char *format, ...)
    {
        werase(win(WindowID::STATUS));
        va_list args;
        va_start(args, format);
        vw_printw(win(WindowID::STATUS), format, args);
        va_end(args);
        wrefresh(win(WindowID::STATUS));
    }

    // DISPLAY FUNCTIONS

    void Context::display_title()
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
        WINDOW *w = win(WindowID::DUNGEON);

        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 16, 48, "a roguelike by connor tynan");

        for (int i = 2; i >= 0; i--)
        {
            wattron(w, COLOR_PAIR(i + 64));
            for (int y = 0; y < 10; y++)
            {
#ifdef USE_WNCURSES
                const wchar_t *line = title_art[i][y];
                for (int x = 0; line[x] != L'\0'; x++)
                    if (line[x] != L' ')
                        mvwaddnwstr(w, 3 + y, x, &line[x], 1);
#else
                const char *line = title_art[i][y];
                for (int x = 0; line[x]; x++)
                    if (line[x] != ' ')
                        mvwaddch(w, 3 + y, x, line[x]);
#endif
            }
            wattroff(w, COLOR_PAIR(i + 64));
        }

        wrefresh(w);
        display_message("Press any key to start...");
    }

    void Context::display_dungeon()
    {
        WINDOW *w = win(WindowID::DUNGEON);
        werase(w);

        for (mapsize_t y = 0; y < game.dungeon.height; y++)
            for (mapsize_t x = 0; x < game.dungeon.width; x++)
            {
                bool visible = game.visibility_at(x, y).visible || !fog_of_war;

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
                    wattron(w, COLOR_PAIR(COLOR_WHITE));

                Dungeon::cell_type_t displayed_cell =
                    fog_of_war
                        ? game.visibility_at(x, y).last_seen
                        : game.dungeon.type_grid.at(x, y);

                mvwaddch(w, y, x, Dungeon::char_map(displayed_cell));

                wattroff(w, A_BOLD);
                if (show_hardness)
                    wattroff(w, COLOR_PAIR(color_idx));
                else
                    wattroff(w, COLOR_PAIR(COLOR_WHITE));
            }

        box(w, 0, 0);
        wrefresh(w);

        display_status("HP: %d  Monsters left: %ld, Tick: %ld",
                       game.player.health,
                       game.filter<Monster>([](const Monster &m)
                                            { return m.active; })
                           .size(),
                       game.current_tick());
    }

    void Context::display_monster_list()
    {
        WINDOW *w = win(WindowID::MONSTER);
        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 0, 2, " Monster List ");
        mvwprintw(w, MONSTER_WIN_HEIGHT - 1, 2, " Press [ESC] to return ");

        int max_visible = MONSTER_WIN_WIDTH - 2;
        std::vector<std::string> lines;

        int global_max_scroll = 0;
        for (const auto &m : game.filtered<Monster>([](const Monster &m)
                                                    { return m.active; }))
        {
            int rel_x = m->x - game.player.x;
            int rel_y = m->y - game.player.y;

            std::string line = std::string(m->name()) + " at " +
                               ((rel_y == 0)
                                    ? ""
                                    : (std::to_string(std::abs(rel_y)) + (rel_y < 0 ? " north" : " south"))) +
                               ((rel_y != 0 && rel_x != 0) ? ", " : "") +
                               ((rel_x == 0)
                                    ? ""
                                    : (std::to_string(std::abs(rel_x)) + (rel_x < 0 ? " west" : " east")));
            lines.push_back(line);

            if (line.length() > (size_t)max_visible)
            {
                int scroll = line.length() - max_visible;
                global_max_scroll = std::max(global_max_scroll, scroll);
            }
        }

        int global_scroll = (2 * static_cast<int>(frame / target_fps)) % (global_max_scroll + 3); // Add 3 to sit at the end of the line

        for (size_t i = vert_scroll, row = 1;
             i < lines.size() && row <= MONSTER_WIN_HEIGHT - 2;
             ++i, ++row)
        {
            mvscrollnstr(w, row, 1, max_visible, global_scroll, "%s", lines[i].c_str());
        }

        wrefresh(w);
    }

    void Context::display_inventory_list()
    {
        WINDOW *w = win(WindowID::INVENTORY);
        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 0, 2, " Inventory ");
        mvwprintw(w, INVENTORY_WIN_HEIGHT - 1, 2, " Press [ESC] to return ");

        for (int i = 0; i < 10; ++i)
        {
            Object &obj = game.player.inventory[i];
            std::string name = !obj.is(Object::TYPE_NONE) ? std::string(obj.name()) : "<empty>";
            mvwprintw(w, 1 + i, 2, "%d. %s", i, name.c_str());
        }

        wrefresh(w);
    }

    void Context::display_equipment_list()
    {
        static const char *slot_names[12] = {
            "Weapon", "Offhand", "Ranged", "Armor", "Helmet", "Cloak",
            "Gloves", "Boots", "Amulet", "Light", "Ring 1", "Ring 2"};

        WINDOW *w = win(WindowID::EQUIPMENT);
        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 0, 2, " Equipment ");
        mvwprintw(w, EQUIPMENT_WIN_HEIGHT - 1, 2, " Press [ESC] to return ");

        for (int i = 0; i < 12; ++i)
        {
            Object &obj = game.player.equipment[i];
            std::string name = !obj.is(Object::TYPE_NONE) ? std::string(obj.name()) : "<empty>";
            mvwprintw(w, 1 + i, 2, "%c. %-8s : %s", 'a' + i, slot_names[i], name.c_str());
        }

        wrefresh(w);
    }

    void Context::display_cursor_select()
    {
        WINDOW *w = win(WindowID::DUNGEON);
        werase(w);

        for (mapsize_t y = 0; y < game.dungeon.height; ++y)
            for (mapsize_t x = 0; x < game.dungeon.width; ++x)
            {
                wattron(w, COLOR_PAIR(COLOR_WHITE));
                if (x == cursor_x && y == cursor_y)
                    mvwaddch(w, y, x, '*');
                else
                {
                    const auto &entity = game.top_entity_at(x, y);
                    if (entity)
                        entity->render(*this, frame);
                    else
                        mvwaddch(w, y, x, Dungeon::char_map(game.dungeon.type_grid.at(x, y)));
                }
            }

        wattroff(w, COLOR_PAIR(COLOR_WHITE));
        box(w, 0, 0);
        wrefresh(w);
    }

    void Context::display_monster_lore_menu()
    {
        WINDOW *w = win(WindowID::LORE);
        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 0, 2, " Monster Description ");
        mvwprintw(w, LORE_WIN_HEIGHT - 1, 2, " Press [ESC] to return ");

        // Name + BOSS
        std::string name = std::string(viewed_monster->name());
        std::string tag = viewed_monster->desc->has_ability(Monster::Abilities::BOSS) ? "  **(BOSS)**" : "";
        std::string name_line = name + tag;
        int name_col = std::max(1, (LORE_WIN_WIDTH - static_cast<int>(name_line.length())) / 2);
        mvwprintw(w, 1, name_col, "%s", name_line.c_str());

        // Stats
        int hp = viewed_monster->health;
        int speed = viewed_monster->speed;
        std::string dmg = viewed_monster->damage.to_string();
        std::string abilities = viewed_monster->abilities_string();

        mvwprintw(w, 2, 2, "HEALTH: %d", hp);
        mvwprintw(w, 2, 26, "ABILITIES: %s", abilities.c_str());
        mvwprintw(w, 3, 2, "SPEED:  %d", speed);
        mvwprintw(w, 3, 26, "DAMAGE: %s", dmg.c_str());

        // Description
        const std::string desc_str = std::string(viewed_monster->description());
        std::vector<std::string> desc_lines;
        std::istringstream iss(desc_str);
        std::string line;
        while (std::getline(iss, line))
        {
            desc_lines.push_back(line);
        }

        int max_lines = LORE_WIN_HEIGHT - 7;
        int max_scroll = std::max(0, static_cast<int>(desc_lines.size()) - max_lines);
        int scroll = std::min(vert_scroll, max_scroll);

        for (int i = 0; i < max_lines && (i + scroll) < (int)desc_lines.size(); ++i)
        {
            mvwprintw(w, 6 + i, 2, "%s", desc_lines[i + scroll].c_str());
        }
        wrefresh(w);
    }

    void Context::display_item_lore_menu()
    {
        WINDOW *w = win(WindowID::LORE);
        werase(w);
        box(w, 0, 0);
        mvwprintw(w, 0, 2, " Item Description ");
        mvwprintw(w, LORE_WIN_HEIGHT - 1, 2, " Press [ESC] to return ");

        if (!viewed_item)
            return;

        const Object &obj = *viewed_item;

        // Centered name + ART flag
        std::string name = std::string(obj.name());
        std::string tag = obj.is_artifact ? "  **(ARTIFACT)**" : "";
        std::string name_line = name + tag;
        int name_col = std::max(1, (LORE_WIN_WIDTH - static_cast<int>(name_line.length())) / 2);
        mvwprintw(w, 1, name_col, "%s", name_line.c_str());

        // Stats — left/right aligned
        mvwprintw(w, 2, 2, "TYPE: %c", object_type_to_char(obj.type));
        mvwprintw(w, 2, 52, "VALUE: %d", obj.value);

        mvwprintw(w, 3, 2, "ATTR:  %d", obj.attribute);
        mvwprintw(w, 3, 26, "SPEED: %d", obj.speed);
        mvwprintw(w, 3, 52, "WEIGHT: %d", obj.weight);

        mvwprintw(w, 4, 2, "HIT:   %d", obj.hit);
        mvwprintw(w, 4, 26, "DAM:   %s", obj.damage.to_string().c_str());
        mvwprintw(w, 4, 52, "DODGE: %d", obj.dodge);

        mvwprintw(w, 5, 2, "DEF:   %d", obj.defense);

        // Description block
        const std::string desc_str = std::string(obj.description());
        std::vector<std::string> desc_lines;
        std::istringstream iss(desc_str);
        std::string line;
        while (std::getline(iss, line))
        {
            desc_lines.push_back(line);
        }

        int max_lines = LORE_WIN_HEIGHT - 7;
        int max_scroll = std::max(0, static_cast<int>(desc_lines.size()) - max_lines);
        int scroll = std::min(vert_scroll, max_scroll);

        for (int i = 0; i < max_lines && (i + scroll) < (int)desc_lines.size(); ++i)
        {
            mvwprintw(w, 6 + i, 2, "%s", desc_lines[i + scroll].c_str());
        }

        wrefresh(w);
    }

    // INPUT HANDLERS (return true if input should advance game state)
    bool Context::handle_dungeon_input(Command cmd, int &dx, int &dy, bool &force)
    {
        switch (cmd)
        {
        case Command::QUIT:
            running = false;
            return true;
        // Movement
        case Command::REST:
            return true;
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
        case Command::STAIRS_UP:
            if (game.dungeon.type_grid.at(game.player.x, game.player.y) == Dungeon::CELL_STAIR_UP)
                game.regenerate_dungeon();
            else
                display_message("You need stairs to get up! ('<')");
            return false;
        case Command::STAIRS_DOWN:
            if (game.dungeon.type_grid.at(game.player.x, game.player.y) == Dungeon::CELL_STAIR_DOWN)
                game.regenerate_dungeon();
            else
                display_message("You need stairs to get down! ('>')");
            return false;

        // Lists
        case Command::SHOW_INVENTORY:
            mode = UIMode::INVENTORY;
            list_open_frame = frame;
            return false;
        case Command::SHOW_EQUIPMENT:
            mode = UIMode::EQUIPMENT;
            list_open_frame = frame;
            return false;
        case Command::SHOW_MONSTER_LIST:
            mode = UIMode::MONSTER_LIST;
            vert_scroll = 0;
            list_open_frame = frame;
            return false;
        case Command::INSPECT_ITEM:
            mode = UIMode::INVENTORY;
            selecting_slot_cmd = cmd;
            vert_scroll = 0;
            list_open_frame = frame;
            return false;
        case Command::DROP_ITEM:
            mode = UIMode::INVENTORY;
            selecting_slot_cmd = cmd;
            vert_scroll = 0;
            list_open_frame = frame;
            return false;
        case Command::EXPUNGE_ITEM:
            mode = UIMode::INVENTORY;
            selecting_slot_cmd = cmd;
            vert_scroll = 0;
            list_open_frame = frame;
            return false;
        case Command::WEAR_ITEM:
            mode = UIMode::INVENTORY;
            selecting_slot_cmd = cmd;
            vert_scroll = 0;
            list_open_frame = frame;
            return false;
        case Command::TAKE_OFF_ITEM:
            mode = UIMode::EQUIPMENT;
            selecting_slot_cmd = cmd;
            vert_scroll = 0;
            list_open_frame = frame;
            return false;

        // cursor select
        case Command::TOGGLE_TELEPORT_MODE:
            mode = UIMode::TELEPORT;
            cursor_x = game.player.x;
            cursor_y = game.player.y;
            display_status("Teleport mode: select location: movement keys+g, random:R. [ESC] to cancel.");
            return false;
        case Command::INSPECT_MONSTER:
            mode = UIMode::TARGETING;
            cursor_x = game.player.x;
            cursor_y = game.player.y;
            display_status("Targeting mode: select monster: movement keys+t. [ESC] to cancel.");
            return false;

        // cheat toggles
        case Command::TOGGLE_FOG_OF_WAR:
            fog_of_war = !fog_of_war;
            return false;
        case Command::TOGGLE_SHOW_HARDNESS:
            show_hardness = !show_hardness;
            return false;

        default:
            return false;
        }
    }

    bool Context::handle_monster_list_input(Command cmd, int &dx, int &dy, bool &force)
    {
        int visible_rows = MONSTER_WIN_HEIGHT - 2;
        size_t alive_monsters = game.filter<Monster>([](const Monster &m)
                                                     { return m.active; })
                                    .size();
        int scroll_max = std::max(0UL, alive_monsters - visible_rows);

        switch (cmd)
        {
        case Command::INFO_SCREEN_SCROLL_DOWN:
            if (vert_scroll < scroll_max)
                vert_scroll++;
            break;
        case Command::INFO_SCREEN_SCROLL_UP:
            if (vert_scroll > 0)
                vert_scroll--;
            break;
        case Command::ESCAPE:
            mode = UIMode::DUNGEON;
            vert_scroll = 0;
            break;
        case Command::QUIT:
            running = false;
            return true;
        default:
            break;
        }

        return false;
    }
    bool Context::handle_inventory_input(Command cmd, int &dx, int &dy, bool &force)
    {
        if (cmd == Command::ESCAPE)
        {
            mode = UIMode::DUNGEON;
            selecting_slot_cmd = Command::NONE;
            return false;
        }
        if (cmd == Command::QUIT)
        {
            running = false;
            selecting_slot_cmd = Command::NONE;
            return true;
        }
        if (selecting_slot_cmd != Command::NONE)
        {
            selecting_slot_cmd = Command::NONE;

            if (cmd == Command::ESCAPE)
                return false;

            int index = static_cast<int>(cmd) - static_cast<int>('0');
            if (index < 0 || index >= 10)
            {
                display_message("Invalid inventory slot.");
                return false;
            }

            Object *obj = &game.player.inventory[index];
            if (obj->is(Object::TYPE_NONE))
            {
                display_message("No item in that slot.");
                return false;
            }

            switch (selecting_slot_cmd)
            {
            case Command::INSPECT_ITEM:
                viewed_item = obj;
                mode = UIMode::LORE_MENU;
                return false;

            case Command::DROP_ITEM:
                game.player.inventory[index] = Object();
                game.add_entity(std::make_unique<ObjectEntity>(obj->to_entity(game.player.x, game.player.y)));
                break;

            case Command::EXPUNGE_ITEM:
                game.player.inventory[index] = Object();
                display_message("Item destroyed.");
                break;

            case Command::WEAR_ITEM:
            {
                if (game.player.equip(index, obj->type))
                    display_message("Item equipped.");
                else
                    display_message("Cannot wear item.");
                break;
            }

            default:
                break;
            }

            mode = UIMode::DUNGEON;
            selecting_slot_cmd = Command::NONE;
            return false;
        }
        return false;
    }

    bool Context::handle_equipment_input(Command cmd, int &dx, int &dy, bool &force)
    {
        if (cmd == Command::ESCAPE)
        {
            mode = UIMode::DUNGEON;
            selecting_slot_cmd = Command::NONE;
            return false;
        }
        if (cmd == Command::QUIT)
        {
            running = false;
            selecting_slot_cmd = Command::NONE;
            return true;
        }
        if (selecting_slot_cmd != Command::NONE)
        {
            selecting_slot_cmd = Command::NONE;

            int index = static_cast<int>(cmd) - static_cast<int>('a');
            if (index < 0 || index >= 12)
            {
                display_message("Invalid equipment slot.");
                return false;
            }

            Object obj = game.player.equipment[index];
            if (obj.is(Object::TYPE_NONE))
            {
                display_message("No item in that slot.");
                return false;
            }

            if (selecting_slot_cmd == Command::TAKE_OFF_ITEM)
            {
                int inv_index = game.player.pickup(&obj);
                if (inv_index == -1)
                {
                    display_message("Inventory full.");
                    return false;
                }
                else
                {
                    game.player.equipment[index] = Object();
                    display_message("Item removed.");
                }
            }

            mode = UIMode::DUNGEON;
            selecting_slot_cmd = Command::NONE;
            return false;
        }

        return false;
    }

    bool Context::handle_cursor_select_input(Command cmd, int &dx, int &dy, bool &force)
    {
        // Movement + Quit
        switch (cmd)
        {
        case Command::MOVE_NW:
            cursor_x--;
            cursor_y--;
            break;
        case Command::MOVE_N:
            cursor_y--;
            break;
        case Command::MOVE_NE:
            cursor_x++;
            cursor_y--;
            break;
        case Command::MOVE_E:
            cursor_x++;
            break;
        case Command::MOVE_SE:
            cursor_x++;
            cursor_y++;
            break;
        case Command::MOVE_S:
            cursor_y++;
            break;
        case Command::MOVE_SW:
            cursor_x--;
            cursor_y++;
            break;
        case Command::MOVE_W:
            cursor_x--;
            break;
        case Command::ESCAPE:
            mode = UIMode::DUNGEON;
            return false;
        case Command::QUIT:
            running = false;
            return true;
        default:
            goto modes;
        }

        // Clamp
        cursor_x = std::clamp(cursor_x, mapsize_t(1), mapsize_t(game.dungeon.width - 2));
        cursor_y = std::clamp(cursor_y, mapsize_t(1), mapsize_t(game.dungeon.height - 2));
        return false;

    modes:
        // Mode-specific behavior
        if (mode == UIMode::TELEPORT)
        {
            switch (cmd)
            {
            case Command::TOGGLE_TELEPORT_MODE:
                if (game.dungeon.hardness_grid.at(cursor_x, cursor_y) < 255)
                {
                    dx = int(cursor_x) - int(game.player.x);
                    dy = int(cursor_y) - int(game.player.y);
                    force = true;
                    mode = UIMode::DUNGEON;
                    return true;
                }
                else
                {
                    display_message("Cannot teleport to this location...");
                }
                break;

            case Command::RANDOM_TELEPORT:
                do
                {
                    cursor_x = rand() % (game.dungeon.width - 2) + 1;
                    cursor_y = rand() % (game.dungeon.height - 2) + 1;
                } while (game.dungeon.hardness_grid.at(cursor_x, cursor_y) >= 255);

                dx = int(cursor_x) - int(game.player.x);
                dy = int(cursor_y) - int(game.player.y);
                force = true;
                mode = UIMode::DUNGEON;
                return true;

            default:
                break;
            }
        }
        else if (mode == UIMode::TARGETING)
        {
            if (cmd == Command::TAKE_OFF_ITEM) // should rename this probably, its t
            {
                for (Entity *e : game.entities_at(cursor_x, cursor_y))
                {
                    Monster *m = e->as<Monster>();
                    if (m && m->active)
                    {
                        viewed_monster = m;
                        vert_scroll = 0;
                        mode = UIMode::LORE_MENU;
                        return false;
                    }
                }

                display_message("No monster here to inspect.");
                return false;
            }
        }
        else
            throw std::runtime_error("Invalid mode for cursor select input");

        return false;
    }

    bool Context::handle_lore_input(Command cmd, int &dx, int &dy, bool &force)
    {
        switch (cmd)
        {
        case Command::INFO_SCREEN_SCROLL_DOWN:
            vert_scroll++;
            break;

        case Command::INFO_SCREEN_SCROLL_UP:
            if (vert_scroll > 0)
                vert_scroll--;
            break;

        case Command::ESCAPE:
            mode = UIMode::DUNGEON;
            vert_scroll = 0;
            break;

        case Command::QUIT:
            running = false;
            return true;

        default:
            break;
        }

        return false;
    }

} // namespace ui
