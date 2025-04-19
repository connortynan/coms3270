#pragma once

#include "entity.hpp"
#include "util/dice.hpp"
#include "util/colors.hpp"

struct ObjectDesc;

class Object : public Entity
{
public:
    enum class Type : unsigned int
    {
        NONE = 0,
        WEAPON = 1 << 0,
        OFFHAND = 1 << 1,
        RANGED = 1 << 2,
        ARMOR = 1 << 3,
        HELMET = 1 << 4,
        CLOAK = 1 << 5,
        GLOVES = 1 << 6,
        BOOTS = 1 << 7,
        RING = 1 << 8,
        AMULET = 1 << 9,
        LIGHT = 1 << 10,
        SCROLL = 1 << 11,
        BOOK = 1 << 12,
        FLASK = 1 << 13,
        GOLD = 1 << 14,
        AMMUNITION = 1 << 15,
        FOOD = 1 << 16,
        WAND = 1 << 17,
        CONTAINER = 1 << 18
    };

    Object(mapsize_t x, mapsize_t y,
           Type type,
           std::vector<short> color,
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

    std::string_view name() const;
    std::string_view description() const;
    bool is(Type t) const
    {
        return (static_cast<unsigned int>(type) & static_cast<unsigned int>(t)) != 0;
    }

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

    Object::Type type = Object::Type::NONE;
    bool is_artifact = false;
};

inline Object::Type object_type_from_string(const std::string &str)
{
    using T = Object::Type;

    if (str == "WEAPON")
        return T::WEAPON;
    if (str == "OFFHAND")
        return T::OFFHAND;
    if (str == "RANGED")
        return T::RANGED;
    if (str == "ARMOR")
        return T::ARMOR;
    if (str == "HELMET")
        return T::HELMET;
    if (str == "CLOAK")
        return T::CLOAK;
    if (str == "GLOVES")
        return T::GLOVES;
    if (str == "BOOTS")
        return T::BOOTS;
    if (str == "RING")
        return T::RING;
    if (str == "AMULET")
        return T::AMULET;
    if (str == "LIGHT")
        return T::LIGHT;
    if (str == "SCROLL")
        return T::SCROLL;
    if (str == "BOOK")
        return T::BOOK;
    if (str == "FLASK")
        return T::FLASK;
    if (str == "GOLD")
        return T::GOLD;
    if (str == "AMMUNITION")
        return T::AMMUNITION;
    if (str == "FOOD")
        return T::FOOD;
    if (str == "WAND")
        return T::WAND;
    if (str == "CONTAINER")
        return T::CONTAINER;

    return Object::Type::NONE;
}
inline char object_type_to_char(Object::Type type)
{
    using T = Object::Type;
    const auto t = static_cast<unsigned int>(type);

    if (t & static_cast<unsigned int>(T::WEAPON))
        return '|';
    if (t & static_cast<unsigned int>(T::OFFHAND))
        return ')';
    if (t & static_cast<unsigned int>(T::RANGED))
        return '}';
    if (t & static_cast<unsigned int>(T::ARMOR))
        return '[';
    if (t & static_cast<unsigned int>(T::HELMET))
        return ']';
    if (t & static_cast<unsigned int>(T::CLOAK))
        return '(';
    if (t & static_cast<unsigned int>(T::GLOVES))
        return '{';
    if (t & static_cast<unsigned int>(T::BOOTS))
        return '\\';
    if (t & static_cast<unsigned int>(T::RING))
        return '=';
    if (t & static_cast<unsigned int>(T::AMULET))
        return '"';
    if (t & static_cast<unsigned int>(T::LIGHT))
        return '_';
    if (t & static_cast<unsigned int>(T::SCROLL))
        return '~';
    if (t & static_cast<unsigned int>(T::BOOK))
        return '?';
    if (t & static_cast<unsigned int>(T::FLASK))
        return '!';
    if (t & static_cast<unsigned int>(T::GOLD))
        return '$';
    if (t & static_cast<unsigned int>(T::AMMUNITION))
        return '/';
    if (t & static_cast<unsigned int>(T::FOOD))
        return ',';
    if (t & static_cast<unsigned int>(T::WAND))
        return '-';
    if (t & static_cast<unsigned int>(T::CONTAINER))
        return '%';

    return '*';
}
