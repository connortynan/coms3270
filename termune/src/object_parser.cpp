#include "object_parser.hpp"
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <string>

#include "util/parser_helpers.hpp"

using namespace ParserHelpers;

void ObjectParser::registerKeywords()
{
    keyword_handlers.clear();

    keyword_handlers["NAME"] = [](auto &o, const std::string &rest, auto &in)
    { return singleLine()(o.name, rest, in); };

    keyword_handlers["DESC"] = [](auto &o, const std::string &, auto &in)
    { return multiLineDescription()(o.description, "", in); };

    keyword_handlers["TYPE"] = [](auto &o, const std::string &rest, auto &in)
    { return singleLine()(o.type, rest, in); };

    keyword_handlers["COLOR"] = [](auto &o, const std::string &rest, auto &in)
    { return splitWords()(o.colors, rest, in); };

    keyword_handlers["WEIGHT"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.weight, rest, in); };

    keyword_handlers["HIT"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.hit, rest, in); };

    keyword_handlers["DAM"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.dam, rest, in); };

    keyword_handlers["DODGE"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.dodge, rest, in); };

    keyword_handlers["DEF"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.def, rest, in); };

    keyword_handlers["SPEED"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.speed, rest, in); };

    keyword_handlers["ATTR"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.attr, rest, in); };

    keyword_handlers["VAL"] = [](auto &o, const std::string &rest, auto &in)
    { return parseDiceField()(o.val, rest, in); };

    keyword_handlers["ART"] = [](auto &o, const std::string &rest, auto &in)
    {
        std::string val = rest;
        std::transform(val.begin(), val.end(), val.begin(), ::toupper);
        if (val == "TRUE")
            o.is_artifact = true;
        else if (val == "FALSE")
            o.is_artifact = false;
        else
            return false;
        return true;
    };

    keyword_handlers["RRTY"] = [](auto &o, const std::string &rest, auto &in)
    { return parseInt()(o.rarity, rest, in); };
}

ObjectDesc ObjectParser::makeDefault() { return ObjectDesc{}; }

bool ObjectParser::validate(const ObjectDesc &o)
{
    return !o.name.empty() &&
           !o.description.empty() &&
           !o.type.empty() &&
           !o.colors.empty() &&
           o.rarity >= 1 && o.rarity <= 100;
}
