#pragma once

#include <curses.h>

enum class ColorPair : short
{
    DEFAULT = -1, // Terminal default color
    BLACK = COLOR_BLACK,
    RED = COLOR_RED,
    GREEN = COLOR_GREEN,
    YELLOW = COLOR_YELLOW,
    BLUE = COLOR_BLUE,
    MAGENTA = COLOR_MAGENTA,
    CYAN = COLOR_CYAN,
    WHITE = COLOR_WHITE
};

// Initializes color pairs: foreground as all standard colors, background as specified (default = DEFAULT)
inline void init_color_pairs(ColorPair background = ColorPair::DEFAULT)
{
    start_color();
    use_default_colors(); // Enable -1 support for transparency

    short bg = static_cast<short>(background);

    for (short fg = COLOR_BLACK; fg <= COLOR_WHITE; ++fg)
    {
        init_pair(fg, fg, bg);
    }
}

struct ColorAttr
{
    ColorAttr(ColorPair cp) : id(static_cast<short>(cp)) {}

    operator int() const
    {
        return COLOR_PAIR(id);
    }

private:
    short id;
};
