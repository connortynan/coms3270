#pragma once

#include "entity.hpp"
#include "util/dice.hpp"

class Character : public Entity
{
public:
    Character(mapsize_t x, mapsize_t y, char sym, const std::vector<short> &cols, int speed, int max_health, const Dice &damage, int zindex = 0)
        : Entity(x, y, sym, cols, zindex), speed(speed), health(max_health), health_max(max_health), damage(damage) {}

    virtual int event_delay() const { return 1000 / speed; }

    virtual ~Character() = default;

    virtual std::string_view name() const = 0;
    virtual std::string_view description() const = 0;

public:
    int speed = 0;
    int health = 0;
    int health_max = 0;
    Dice damage;
};
