#pragma once

#include <string>
#include <sstream>
#include <functional>
#include <vector>
#include <algorithm>
#include "../util/dice.hpp"

namespace ParserHelpers
{
    // Trim trailing whitespace (including carriage returns)
    inline void trim(std::string &s)
    {
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    }

    // Single-line string fields like NAME, SYMB, TYPE
    inline std::function<bool(std::string &, const std::string &, std::istream &)> singleLine()
    {
        return [](std::string &out, const std::string &rest, std::istream &)
        {
            out = rest;
            return !out.empty();
        };
    }

    // Integer fields like RRTY
    inline std::function<bool(int &, const std::string &, std::istream &)> parseInt()
    {
        return [](int &out, const std::string &rest, std::istream &)
        {
            std::string trimmed = rest;
            trim(trimmed);
            try
            {
                out = std::stoi(trimmed);
                return true;
            }
            catch (...)
            {
                return false;
            }
        };
    }

    // Space-delimited fields like COLOR, ABIL
    inline std::function<bool(std::vector<std::string> &, const std::string &, std::istream &)> splitWords()
    {
        return [](std::vector<std::string> &out, const std::string &rest, std::istream &)
        {
            std::istringstream iss(rest);
            std::string token;
            while (iss >> token)
                out.push_back(token);
            return !out.empty();
        };
    }

    // Dice fields like SPEED, HP, DAM, etc.
    inline std::function<bool(Dice &, const std::string &, std::istream &)> parseDiceField()
    {
        return [](Dice &out, const std::string &rest, std::istream &)
        {
            std::string trimmed = rest;
            trim(trimmed);
            try
            {
                out = parseDice(trimmed);
                return true;
            }
            catch (...)
            {
                return false;
            }
        };
    }

    // Multi-line DESC field, terminated by a line with only "."
    inline std::function<bool(std::string &, const std::string &, std::istream &)> multiLineDescription()
    {
        return [](std::string &out, const std::string &, std::istream &in)
        {
            std::ostringstream oss;
            std::string line;
            while (std::getline(in, line))
            {
                std::string trimmed = line;
                trim(trimmed);
                if (trimmed == ".")
                    break;
                oss << line << '\n';
            }
            out = oss.str();
            return !out.empty();
        };
    }
}
