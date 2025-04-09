#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "monster_parser.h"
#include "object_parser.h"

void printMonster(const MonsterDesc &m)
{
    std::cout << m.name << "\n";
    std::cout << m.description;
    std::cout << m.symbol << "\n";

    for (const auto &c : m.colors)
        std::cout << c << " ";
    std::cout << "\n";

    printDice(m.speed);

    for (const auto &a : m.abilities)
        std::cout << a << " ";
    std::cout << "\n";

    printDice(m.hp);
    printDice(m.dam);

    std::cout << m.rarity << "\n\n";
}

void printObject(const ObjectDesc &o)
{
    std::cout << o.name << "\n";
    std::cout << o.description;
    std::cout << o.type << "\n";

    for (const auto &c : o.colors)
        std::cout << c << " ";
    std::cout << "\n";

    printDice(o.hit);
    printDice(o.dam);
    printDice(o.dodge);
    printDice(o.def);
    printDice(o.weight);
    printDice(o.speed);
    printDice(o.attr);
    printDice(o.val);

    std::cout << (o.is_artifact ? "TRUE" : "FALSE") << "\n";
    std::cout << o.rarity << "\n\n";
}

std::string getRLGPath(const std::string &filename)
{
    const char *home = getenv("HOME");
    if (!home)
        throw std::runtime_error("Could not determine HOME directory.");
    return std::string(home) + "/.rlg327/" + filename;
}

int main()
{
    try
    {
        std::ifstream monsterFile(getRLGPath("monster_desc.txt"));
        if (monsterFile)
        {
            MonsterParser mparser(monsterFile, "BEGIN MONSTER", "RLG327 MONSTER DESCRIPTION 1");
            auto monsters = mparser.parseAll();
            for (const auto &m : monsters)
                printMonster(m);
        }

        std::ifstream objectFile(getRLGPath("object_desc.txt"));
        if (objectFile)
        {
            ObjectParser oparser(objectFile, "BEGIN OBJECT", "RLG327 OBJECT DESCRIPTION 1");
            auto objects = oparser.parseAll();
            for (const auto &o : objects)
                printObject(o);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
