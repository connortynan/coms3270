#pragma once

#include <string>
#include <stdexcept>
#include <cstdio>
#include <iostream>

#pragma once

#include <string>
#include <stdexcept>
#include <cstdio>
#include <iostream>
#include <random>

struct Dice
{
    int base = 0;
    int count = 0;
    int sides = 0;

    Dice() = default;
    Dice(int base, int count, int sides) : base(base), count(count), sides(sides) {}

    static Dice parse(const std::string &s)
    {
        Dice d;
        if (sscanf(s.c_str(), "%d+%dd%d", &d.base, &d.count, &d.sides) != 3)
        {
            throw std::runtime_error("Invalid dice format: " + s);
        }
        return d;
    }

    std::string to_string() const
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d+%dd%d", base, count, sides);
        return std::string(buffer);
    }

    void print(std::ostream &out = std::cout) const
    {
        out << base << "+" << count << "d" << sides;
    }

    int roll() const
    {
        static thread_local std::mt19937 rng(std::random_device{}());
        return roll(rng);
    }

    template <typename RNG>
    int roll(RNG &rng) const
    {
        int total = base;
        if (count > 0 && sides > 0)
        {
            std::uniform_int_distribution<> dist(1, sides);
            for (int i = 0; i < count; ++i)
            {
                total += dist(rng);
            }
        }
        return total;
    }
};
