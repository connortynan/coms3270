#pragma once

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "util/parser.hpp"
#include "util/dice.hpp"
#include "util/colors.hpp"

#include "monster.hpp"

struct MonsterDesc
{
    std::string name;
    std::string description;
    char symbol = '\0';
    std::vector<short> colors;
    Dice speed, hp, dam;
    Monster::Abilities abilities = Monster::Abilities::NONE;
    int rarity = -1;

    bool has_ability(Monster::Abilities ability) const
    {
        return (static_cast<unsigned int>(abilities) & static_cast<unsigned int>(ability)) != 0;
    }

public:
    std::unique_ptr<Monster> make_instance(mapsize_t x, mapsize_t y, std::mt19937 &rng) const;
};

class MonsterParser : public Parser<MonsterDesc>
{
public:
    using Parser::Parser;

    static std::vector<MonsterDesc> load_from_file(const std::string &filename)
    {
        std::ifstream in(filename);
        if (!in)
            throw std::runtime_error("Could not open file: " + filename);

        MonsterParser parser(in, "BEGIN MONSTER", "RLG327 MONSTER DESCRIPTION 1");
        return parser.parseAll();
    }

protected:
    void registerKeywords() override;
    MonsterDesc makeDefault() override;
    bool validate(const MonsterDesc &) override;
};

inline Monster::Abilities ability_from_string(const std::string &str)
{
    if (str == "SMART")
        return Monster::Abilities::INTELLIGENT;
    if (str == "TELE")
        return Monster::Abilities::TELEPATHIC;
    if (str == "TUNNEL")
        return Monster::Abilities::TUNNELING;
    if (str == "ERRATIC")
        return Monster::Abilities::ERRATIC;
    if (str == "PASS")
        return Monster::Abilities::PASS;
    if (str == "DESTROY")
        return Monster::Abilities::DESTROY;
    if (str == "PICKUP")
        return Monster::Abilities::PICKUP;
    if (str == "UNIQ")
        return Monster::Abilities::UNIQUE;
    if (str == "BOSS")
        return Monster::Abilities::BOSS;
    return Monster::Abilities::NONE;
}