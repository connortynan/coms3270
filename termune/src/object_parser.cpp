#include "object_parser.hpp"
#include <sstream>
#include <iostream>
#include "util/parser_helpers.hpp"

#include "object_item.hpp"

using namespace ParserHelpers;

void ObjectParser::registerKeywords()
{
    keyword_handlers.clear();

    keyword_handlers["NAME"] = [](auto &o, const std::string &rest, auto &in)
    {
        return singleLine()(o.name, rest, in);
    };

    keyword_handlers["DESC"] = [](auto &o, const std::string &rest, auto &in)
    {
        return multiLineDescription()(o.description, rest, in);
    };

    keyword_handlers["TYPE"] = [](auto &o, const std::string &rest, auto &in)
    {
        std::vector<std::string> tokens;
        splitWords()(tokens, rest, in);

        o.type = Object::TYPE_NONE;
        for (const std::string &token : tokens)
        {
            auto t = object_type_from_string(token);
            if (t == Object::TYPE_NONE)
            {
                std::cerr << "Invalid TYPE: " << token << '\n';
                return false;
            }
            o.type = static_cast<Object::Type>(
                static_cast<uint32_t>(o.type) |
                static_cast<uint32_t>(t));
        }
        return true;
    };

    keyword_handlers["COLOR"] = [](auto &o, const std::string &rest, auto &in)
    {
        std::vector<std::string> names;
        splitWords()(names, rest, in);
        o.colors.clear();
        for (const auto &name : names)
        {
            short c = color_from_string(name);
            if (c == -1)
            {
                std::cerr << "Invalid COLOR: " << name << '\n';
                return false;
            }
            o.colors.push_back(c);
        }
        return true;
    };

    keyword_handlers["WEIGHT"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.weight, rest, in);
    };

    keyword_handlers["HIT"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.hit, rest, in);
    };

    keyword_handlers["DAM"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.dam, rest, in);
    };

    keyword_handlers["DODGE"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.dodge, rest, in);
    };

    keyword_handlers["DEF"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.def, rest, in);
    };

    keyword_handlers["SPEED"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.speed, rest, in);
    };

    keyword_handlers["ATTR"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.attr, rest, in);
    };

    keyword_handlers["VAL"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseDiceField()(o.val, rest, in);
    };

    keyword_handlers["ART"] = [](auto &o, const std::string &rest, auto &in)
    {
        std::string val;
        if (!singleLine()(val, rest, in))
            return false;
        if (val == "TRUE")
            o.is_artifact = true;
        else if (val == "FALSE")
            o.is_artifact = false;
        else
        {
            std::cerr << "Invalid ART value: " << val << '\n';
            return false;
        }
        return true;
    };

    keyword_handlers["RRTY"] = [](auto &o, const std::string &rest, auto &in)
    {
        return parseInt()(o.rarity, rest, in);
    };
}

ObjectDesc ObjectParser::makeDefault()
{
    return ObjectDesc{};
}

bool ObjectParser::validate(const ObjectDesc &o)
{
    return !o.name.empty() &&
           !o.description.empty() &&
           o.type != Object::TYPE_NONE &&
           !o.colors.empty() &&
           o.rarity >= 1 && o.rarity <= 100;
}

std::unique_ptr<ObjectEntity> ObjectDesc::make_instance(mapsize_t x, mapsize_t y, std::mt19937 &rng) const
{
    auto obj = std::make_unique<ObjectEntity>(
        x, y, type, colors,
        is_artifact, this,
        weight.roll(rng), hit.roll(rng), dam, dodge.roll(rng),
        def.roll(rng), speed.roll(rng), attr.roll(rng), val.roll(rng));

    return obj;
}
