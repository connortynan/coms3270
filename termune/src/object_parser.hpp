#pragma once

#include <string>
#include <vector>
#include <memory>

#include "util/parser.hpp"
#include "util/dice.hpp"
#include "util/colors.hpp"
#include "object_entity.hpp"
#include "types.hpp"

struct ObjectDesc
{
    std::string name;
    std::string description;
    Object::Type type = Object::TYPE_NONE;
    std::vector<short> colors;

    Dice weight, hit, dam, dodge, def, speed, attr, val;

    bool is_artifact = false;
    int rarity = -1;

public:
    std::unique_ptr<ObjectEntity> make_instance(mapsize_t x, mapsize_t y, std::mt19937 &rng) const;
};

class ObjectParser : public Parser<ObjectDesc>
{
public:
    using Parser::Parser;

    static std::vector<ObjectDesc> load_from_file(const std::string &filename)
    {
        std::ifstream in(filename);
        if (!in)
            throw std::runtime_error("Could not open file: " + filename);

        ObjectParser parser(in, "BEGIN OBJECT", "RLG327 OBJECT DESCRIPTION 1");
        return parser.parseAll();
    }

protected:
    void registerKeywords() override;
    ObjectDesc makeDefault() override;
    bool validate(const ObjectDesc &o) override;
};
