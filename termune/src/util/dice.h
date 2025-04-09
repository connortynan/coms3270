#pragma once

#include <string>
#include <stdexcept>
#include <cstdio>
#include <iostream>

struct Dice
{
    int base = 0;
    int count = 0;
    int sides = 0;
};

inline Dice parseDice(const std::string &s)
{
    Dice d;
    if (sscanf(s.c_str(), "%d+%dd%d", &d.base, &d.count, &d.sides) != 3)
    {
        throw std::runtime_error("Invalid dice format: " + s);
    }
    return d;
}

inline void printDice(const Dice &d)
{
    std::cout << d.base << "+" << d.count << "d" << d.sides << "\n";
}
