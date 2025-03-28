#pragma once

namespace utils
{
    template <typename T>
    constexpr T clamp(T x, T min, T max)
    {
        return std::max(min, std::min(x, max));
    }

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
