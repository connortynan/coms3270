#include "monster_parser.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstdio>

#include "util/parser_helpers.hpp"

using namespace ParserHelpers;

void MonsterParser::registerKeywords()
{
    keyword_handlers.clear();

    keyword_handlers["NAME"] = [](auto &m, const std::string &rest, auto &in)
    { return singleLine()(m.name, rest, in); };

    keyword_handlers["SYMB"] = [](auto &m, const std::string &rest, auto &in)
    {
        std::string line;
        if (!singleLine()(line, rest, in))
            return false;
        if (line.empty())
            return false;

        m.symbol = line[0]; // just take the first character
        return true;
    };

    keyword_handlers["COLOR"] = [](auto &m, const std::string &rest, auto &in)
    {
        std::vector<std::string> names;
        splitWords()(names, rest, in);

        m.colors.clear();
        for (const std::string &name : names)
        {
            short color = color_from_string(name);
            if (color == -1)
            {
                // If the color is not recognized, return false
                std::cerr << "Unknown color: " << color << std::endl;
                return false;
            }
            m.colors.push_back(color);
        }
        return true;
    };

    keyword_handlers["SPEED"] = [](auto &m, const std::string &rest, auto &in)
    { return parseDiceField()(m.speed, rest, in); };

    keyword_handlers["HP"] = [](auto &m, const std::string &rest, auto &in)
    { return parseDiceField()(m.hp, rest, in); };

    keyword_handlers["DAM"] = [](auto &m, const std::string &rest, auto &in)
    { return parseDiceField()(m.dam, rest, in); };

    keyword_handlers["RRTY"] = [](auto &m, const std::string &rest, auto &in)
    { return parseInt()(m.rarity, rest, in); };

    keyword_handlers["ABIL"] = [](auto &m, const std::string &rest, auto &in)
    {
        std::vector<std::string> tokens;
        splitWords()(tokens, rest, in);

        m.abilities = Monster::Abilities::NONE;
        for (const auto &token : tokens)
        {
            Monster::Abilities ability = ability_from_string(token);
            if (ability == Monster::Abilities::NONE)
            {
                // If the ability is not recognized, return false
                std::cerr << "Unknown ability: " << token << std::endl;
                return false;
            }
            m.abilities = static_cast<Monster::Abilities>(
                static_cast<uint32_t>(m.abilities) |
                static_cast<uint32_t>(ability));
        }
        return true;
    };

    keyword_handlers["DESC"] = [](auto &m, const std::string &rest, auto &in)
    { return multiLineDescription()(m.description, rest, in); };
}

MonsterDesc MonsterParser::makeDefault() { return MonsterDesc{}; }

bool MonsterParser::validate(const MonsterDesc &m)
{
    return !m.name.empty() &&
           !m.description.empty() &&
           m.symbol != '\0' &&
           !m.colors.empty() &&
           m.rarity >= 1 && m.rarity <= 100;
}

std::unique_ptr<Monster> MonsterDesc::make_instance(mapsize_t x, mapsize_t y, std::mt19937 &rng) const
{
    auto m = std::make_unique<Monster>(
        x, y,
        symbol,
        colors,
        speed.roll(),
        hp.roll(),
        abilities,
        this,
        dam);

    return m;
}