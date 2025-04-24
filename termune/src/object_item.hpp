#pragma once

#include <string>
#include <vector>

#include "util/dice.hpp"
#include "types.hpp"

class ObjectDesc;
class ObjectEntity;

class Object
{
public:
    enum Type : unsigned int
    {
        TYPE_NONE = 0,
        TYPE_WEAPON = 1 << 0,
        TYPE_OFFHAND = 1 << 1,
        TYPE_RANGED = 1 << 2,
        TYPE_ARMOR = 1 << 3,
        TYPE_HELMET = 1 << 4,
        TYPE_CLOAK = 1 << 5,
        TYPE_GLOVES = 1 << 6,
        TYPE_BOOTS = 1 << 7,
        TYPE_RING = 1 << 8,
        TYPE_AMULET = 1 << 9,
        TYPE_LIGHT = 1 << 10,
        TYPE_SCROLL = 1 << 11,
        TYPE_BOOK = 1 << 12,
        TYPE_FLASK = 1 << 13,
        TYPE_GOLD = 1 << 14,
        TYPE_AMMUNITION = 1 << 15,
        TYPE_FOOD = 1 << 16,
        TYPE_WAND = 1 << 17,
        TYPE_CONTAINER = 1 << 18
    };

public:
    Object(Object::Type type,
           bool is_artifact,
           const ObjectDesc *desc,
           int weight,
           int hit,
           const Dice &damage,
           int dodge,
           int defense,
           int speed,
           int attribute,
           int value);
    Object() : Object(Object::TYPE_NONE, false, nullptr, 0, 0, Dice(), 0, 0, 0, 0, 0) {}

    virtual std::string_view name() const;
    virtual std::string_view description() const;
    bool is(Type t) const;

    ObjectEntity to_entity(mapsize_t x, mapsize_t y) const;

public:
    const ObjectDesc *desc = nullptr;

    int weight = 0;
    int hit = 0;
    Dice damage;
    int dodge = 0;
    int defense = 0;
    int speed = 0;
    int attribute = 0;
    int value = 0;

    Object::Type type = Object::TYPE_NONE;
    bool is_artifact = false;
};

inline Object::Type object_type_from_string(const std::string &str)
{
    if (str == "WEAPON")
        return Object::TYPE_WEAPON;
    if (str == "OFFHAND")
        return Object::TYPE_OFFHAND;
    if (str == "RANGED")
        return Object::TYPE_RANGED;
    if (str == "ARMOR")
        return Object::TYPE_ARMOR;
    if (str == "HELMET")
        return Object::TYPE_HELMET;
    if (str == "CLOAK")
        return Object::TYPE_CLOAK;
    if (str == "GLOVES")
        return Object::TYPE_GLOVES;
    if (str == "BOOTS")
        return Object::TYPE_BOOTS;
    if (str == "RING")
        return Object::TYPE_RING;
    if (str == "AMULET")
        return Object::TYPE_AMULET;
    if (str == "LIGHT")
        return Object::TYPE_LIGHT;
    if (str == "SCROLL")
        return Object::TYPE_SCROLL;
    if (str == "BOOK")
        return Object::TYPE_BOOK;
    if (str == "FLASK")
        return Object::TYPE_FLASK;
    if (str == "GOLD")
        return Object::TYPE_GOLD;
    if (str == "AMMUNITION")
        return Object::TYPE_AMMUNITION;
    if (str == "FOOD")
        return Object::TYPE_FOOD;
    if (str == "WAND")
        return Object::TYPE_WAND;
    if (str == "CONTAINER")
        return Object::TYPE_CONTAINER;

    return Object::TYPE_NONE;
}
inline char object_type_to_char(Object::Type type)
{
    const auto t = static_cast<unsigned int>(type);

    if (t & static_cast<unsigned int>(Object::TYPE_WEAPON))
        return '|';
    if (t & static_cast<unsigned int>(Object::TYPE_OFFHAND))
        return ')';
    if (t & static_cast<unsigned int>(Object::TYPE_RANGED))
        return '}';
    if (t & static_cast<unsigned int>(Object::TYPE_ARMOR))
        return '[';
    if (t & static_cast<unsigned int>(Object::TYPE_HELMET))
        return ']';
    if (t & static_cast<unsigned int>(Object::TYPE_CLOAK))
        return '(';
    if (t & static_cast<unsigned int>(Object::TYPE_GLOVES))
        return '{';
    if (t & static_cast<unsigned int>(Object::TYPE_BOOTS))
        return '\\';
    if (t & static_cast<unsigned int>(Object::TYPE_RING))
        return '=';
    if (t & static_cast<unsigned int>(Object::TYPE_AMULET))
        return '"';
    if (t & static_cast<unsigned int>(Object::TYPE_LIGHT))
        return '_';
    if (t & static_cast<unsigned int>(Object::TYPE_SCROLL))
        return '~';
    if (t & static_cast<unsigned int>(Object::TYPE_BOOK))
        return '?';
    if (t & static_cast<unsigned int>(Object::TYPE_FLASK))
        return '!';
    if (t & static_cast<unsigned int>(Object::TYPE_GOLD))
        return '$';
    if (t & static_cast<unsigned int>(Object::TYPE_AMMUNITION))
        return '/';
    if (t & static_cast<unsigned int>(Object::TYPE_FOOD))
        return ',';
    if (t & static_cast<unsigned int>(Object::TYPE_WAND))
        return '-';
    if (t & static_cast<unsigned int>(Object::TYPE_CONTAINER))
        return '%';

    return '*';
}
