#pragma once

namespace ui
{
    enum class Command
    {
        NONE,
        ESCAPE,
        QUIT,

        // movement
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

        // inspect
        INSPECT_MONSTER,
        INSPECT_ITEM,

        // info screens
        SHOW_MONSTER_LIST,
        SHOW_INVENTORY,
        SHOW_EQUIPMENT,

        INFO_SCREEN_SCROLL_UP,
        INFO_SCREEN_SCROLL_DOWN,

        // item management
        WEAR_ITEM,
        TAKE_OFF_ITEM,
        DROP_ITEM,
        EXPUNGE_ITEM,

        // cheats
        TOGGLE_FOG_OF_WAR,
        TOGGLE_TELEPORT_MODE,
        RANDOM_TELEPORT,
        TOGGLE_SHOW_HARDNESS
    };

    inline Command get_command_from_key(int ch, bool raw = false)
    {
        // Always prioritize ESC and Q, even in raw mode
        if (ch == 27) // ASCII ESC
            return Command::ESCAPE;
        if (ch == 'Q')
            return Command::QUIT;

        if (raw)
            return static_cast<Command>(ch); // raw input, no mapping (cast back to char)

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
            return Command::SHOW_MONSTER_LIST;
        case 0403: // KEY_UP
            return Command::INFO_SCREEN_SCROLL_UP;
        case 0402: // KEY_DOWN
            return Command::INFO_SCREEN_SCROLL_DOWN;
        case 27: // ESC
            return Command::ESCAPE;
        case 'i':
            return Command::SHOW_INVENTORY;
        case 'e':
            return Command::SHOW_EQUIPMENT;
        case 'w':
            return Command::WEAR_ITEM;
        case 't':
            return Command::TAKE_OFF_ITEM;
        case 'L':
            return Command::INSPECT_MONSTER;
        case 'd':
            return Command::DROP_ITEM;
        case 'x':
            return Command::EXPUNGE_ITEM;
        case 'I':
            return Command::INSPECT_ITEM;
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

} // namespace ui
