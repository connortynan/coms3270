#pragma once

#include "entity.hpp"

class Character : public Entity
{
public:
    Character(mapsize_t x, mapsize_t y, char sym, const std::vector<ColorAttr> &cols, int zindex = 0)
        : Entity(x, y, sym, colors, zindex) {}
    Character(mapsize_t x, mapsize_t y, char sym, ColorAttr col, int zindex = 0)
        : Entity(x, y, sym, col, zindex) {}

    virtual ~Character() = default;
};
