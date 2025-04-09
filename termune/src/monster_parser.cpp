#include "monster_parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cstdio>

#include "util/parser_helpers.h"

using namespace ParserHelpers;

void MonsterParser::registerKeywords()
{
    keyword_handlers.clear();

    keyword_handlers["NAME"] = [](auto &m, const std::string &rest, auto &in)
    { return singleLine()(m.name, rest, in); };

    keyword_handlers["SYMB"] = [](auto &m, const std::string &rest, auto &in)
    { return singleLine()(m.symbol, rest, in); };

    keyword_handlers["COLOR"] = [](auto &m, const std::string &rest, auto &in)
    { return splitWords()(m.colors, rest, in); };

    keyword_handlers["SPEED"] = [](auto &m, const std::string &rest, auto &in)
    { return parseDiceField()(m.speed, rest, in); };

    keyword_handlers["HP"] = [](auto &m, const std::string &rest, auto &in)
    { return parseDiceField()(m.hp, rest, in); };

    keyword_handlers["DAM"] = [](auto &m, const std::string &rest, auto &in)
    { return parseDiceField()(m.dam, rest, in); };

    keyword_handlers["RRTY"] = [](auto &m, const std::string &rest, auto &in)
    { return parseInt()(m.rarity, rest, in); };

    keyword_handlers["ABIL"] = [](auto &m, const std::string &rest, auto &in)
    { return splitWords()(m.abilities, rest, in); };

    keyword_handlers["DESC"] = [](auto &m, const std::string &rest, auto &in)
    { return multiLineDescription()(m.description, rest, in); };
}

MonsterDesc MonsterParser::makeDefault() { return MonsterDesc{}; }

bool MonsterParser::validate(const MonsterDesc &m)
{
    return !m.name.empty() &&
           !m.description.empty() &&
           !m.symbol.empty() &&
           !m.colors.empty() &&
           m.rarity >= 1 && m.rarity <= 100;
}
