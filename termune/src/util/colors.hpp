#pragma once

#include <ncurses.h>
#include <string>

inline short color_from_string(const std::string &str)
{
    if (str == "BLACK")
        return COLOR_BLACK;
    if (str == "RED")
        return COLOR_RED;
    if (str == "GREEN")
        return COLOR_GREEN;
    if (str == "YELLOW")
        return COLOR_YELLOW;
    if (str == "BLUE")
        return COLOR_BLUE;
    if (str == "MAGENTA")
        return COLOR_MAGENTA;
    if (str == "CYAN")
        return COLOR_CYAN;
    if (str == "WHITE")
        return COLOR_WHITE;
    return -1;
}

inline void init_color_pairs(short background = -1)
{
    start_color();
    use_default_colors(); // allows -1 for transparency

    for (short i = 0; i < 8; ++i)
        init_pair(i, i, background);
}
