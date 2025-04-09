#pragma once

#include <string>
#include <vector>

#include "util/parser.hpp"
#include "util/dice.hpp"

struct ObjectDesc
{
    std::string name;
    std::string description;
    std::string type;
    std::vector<std::string> colors;
    Dice weight, hit, dam, dodge, def, speed, attr, val;
    bool is_artifact = false;
    int rarity = -1;
};

class ObjectParser : public Parser<ObjectDesc>
{
public:
    using Parser::Parser;

protected:
    void registerKeywords() override;
    ObjectDesc makeDefault() override;
    bool validate(const ObjectDesc &) override;
};
