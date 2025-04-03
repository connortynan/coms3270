#pragma once

#include <limits>

#include "types.h"

constexpr char DEFAULT_PLAYER_SPEED = 10;

class UiManager;
class GameContext;

constexpr entity_id_t ENTITY_NONE = std::numeric_limits<entity_id_t>::max();
constexpr entity_id_t ENTITY_PLAYER = 0;

class Entity
{
public:
    Entity(mapsize_t x, mapsize_t y, char speed, char display_char, entity_id_t id = ENTITY_NONE) : x(x), y(y), speed(speed), id(id), ch(display_char) {}
    virtual ~Entity() {}

    virtual bool move(int dx, int dy, GameContext &g, bool force = false);

public:
    mapsize_t x;
    mapsize_t y;
    char speed;
    bool alive = true;
    entity_id_t id;
    char ch;
};

class Player : public Entity
{
public:
    Player(mapsize_t x = 0, mapsize_t y = 0) : Entity(x, y, DEFAULT_PLAYER_SPEED, '@', ENTITY_PLAYER) {}
};

class Monster : public Entity
{
public:
    static constexpr mapsize_t EMPTY_TARGET = std::numeric_limits<mapsize_t>::max();

    Monster(mapsize_t x, mapsize_t y, char speed, entity_id_t id,
            bool intelligent, bool telepathic, bool erratic, bool tunneling)
        : Entity(x, y, speed, '?', id),
          intelligent(intelligent),
          telepathic(telepathic),
          erratic(erratic),
          tunneling(tunneling)
    {
        ch = "0123456789abcdef"[characteristics()];
    }

    Monster(mapsize_t x, mapsize_t y, char speed, entity_id_t id, int flags) : Entity(x, y, speed, "0123456789abcdef"[flags], id)
    {
        intelligent = flags & 1;
        telepathic = (flags >> 1) & 1;
        tunneling = (flags >> 2) & 1;
        erratic = (flags >> 1) & 1;
    }

    // Puts characteristics in 4 bit integer in order: ETLI
    unsigned char characteristics() const
    {
        return (erratic << 3) | (tunneling << 2) | (telepathic << 1) | intelligent;
    }

    void get_desired_move(int &dx, int &dy, GameContext &g) const;
    bool move(int dx, int dy, GameContext &g, bool force = false);

    static void update_global_maps(GameContext &game);

private:
    mapsize_t target_x = EMPTY_TARGET;
    mapsize_t target_y = EMPTY_TARGET;
    bool has_line_of_sight = false;

    bool intelligent;
    bool telepathic;
    bool erratic;
    bool tunneling;
};