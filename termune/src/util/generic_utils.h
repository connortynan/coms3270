#pragma once

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
} // namespace utils
