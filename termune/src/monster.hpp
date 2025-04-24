#pragma once

#include <limits>

#include "character.hpp"

constexpr int MONSTER_ZINDEX = 2; // Above items, below player

struct MonsterDesc;
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
        TUNNELING = 8,
        PASS = 16,
        PICKUP = 32,
        DESTROY = 64,
        UNIQUE = 128,
        BOSS = 256
    };

    Monster(mapsize_t x, mapsize_t y,
            char symbol,
            std::vector<short> color,
            int speed,
            int max_health,
            Abilities abilities,
            const MonsterDesc *desc,
            const Dice &dmg)
        : Character(x, y, symbol, color, speed, max_health, dmg, MONSTER_ZINDEX),
          desc(desc),
          abilities(abilities) {}

    void get_desired_move(int &dx, int &dy, bool &force, GameContext &g) const;
    virtual bool move(int dx, int dy, GameContext &g, bool force = false) override;

    bool has(Abilities ability) const
    {
        return (static_cast<unsigned int>(abilities) & static_cast<unsigned int>(ability)) != 0;
    }

    std::string_view name() const override;
    std::string_view description() const override;

    void on_collision(Entity &other) override;

    void update_sight(mapsize_t x, mapsize_t y, GameContext &g);

    std::string abilities_string() const
    {
        std::string result;
        if (has(Abilities::INTELLIGENT))
            result += "SMART, ";
        if (has(Abilities::TELEPATHIC))
            result += "TELE, ";
        if (has(Abilities::ERRATIC))
            result += "ERRATIC, ";
        if (has(Abilities::TUNNELING))
            result += "TUNNEL, ";
        if (has(Abilities::PASS))
            result += "PASS, ";
        if (has(Abilities::PICKUP))
            result += "PICKUP, ";
        if (has(Abilities::DESTROY))
            result += "DESTROY, ";

        if (result.length() < 2)
            return "<none>";
        else
            return result.substr(0, result.length() - 2); // Remove last comma and space
    }

public:
    const MonsterDesc *desc = nullptr;
    Abilities abilities;

private:
    bool has_line_of_sight = false;
    mapsize_t target_x = EMPTY_TARGET;
    mapsize_t target_y = EMPTY_TARGET;
};
