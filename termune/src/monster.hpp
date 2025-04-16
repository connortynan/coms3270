#pragma once

#include <limits>

#include "character.hpp"

class Monster : public Character
{
public:
    static constexpr mapsize_t EMPTY_TARGET = std::numeric_limits<mapsize_t>::max();

    enum class Abilities : unsigned int
    {
        NONE = 0,
        INTELLIGENT = 1,
        TELEPATHIC = 2,
        ERRATIC = 4,
        TUNNELING = 8
    };

    Monster(mapsize_t x, mapsize_t y, char symbol, std::vector<ColorAttr> col,
            Abilities abilities = Abilities::NONE, int zindex = 0)
        : Character(x, y, symbol, col, zindex), abilities(abilities) {}

    void get_desired_move(int &dx, int &dy, GameContext &g) const;
    virtual bool move(int dx, int dy, GameContext &g, bool force = false) override;

public:
    Abilities abilities;
    mapsize_t target_x = EMPTY_TARGET;
    mapsize_t target_y = EMPTY_TARGET;
};
