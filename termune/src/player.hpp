#pragma once

#include "character.hpp"
#include "util/colors.hpp"

class Player : public Character
{
public:
    Player(mapsize_t x, mapsize_t y)
        : Character(x, y, '@', ColorPair::WHITE) {}
};