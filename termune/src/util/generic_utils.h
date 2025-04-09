#pragma once

#include <string>

namespace utils
{
    template <typename T>
    constexpr T lerp(T t, T a, T b)
    {
        return a + (b - a) * t;
    }

    template <typename T>
    constexpr T dot2(T ax, T ay, T bx, T by)
    {
        return ax * bx + ay * by;
    }

    inline std::string trim(const std::string &s)
    {
        const auto start = s.find_first_not_of(" \t\r\n");
        const auto end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }
} // namespace utils
