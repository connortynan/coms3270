#pragma once

#include <string>
#include <vector>

#include "util/parser.h"
#include "util/dice.h"

struct MonsterDesc
{
    std::string name;
    std::string description;
    std::string symbol;
    std::vector<std::string> colors;
    Dice speed, hp, dam;
    std::vector<std::string> abilities;
    int rarity = -1;
};

class MonsterParser : public Parser<MonsterDesc>
{
public:
    using Parser::Parser;

protected:
    void registerKeywords() override;
    MonsterDesc makeDefault() override;
    bool validate(const MonsterDesc &) override;
};
