#include "noise.h"

#include <array>
#include <random>
#include <numeric>
#include <algorithm>
#include <cmath>

#include "util/generic_utils.h"

constexpr int TableSize = 512;

std::array<uint8_t, TableSize * 2> perm;

namespace Noise
{
    std::array<uint8_t, TableSize * 2> perm;

    void generate_permutation(unsigned int seed = 0)
    {
        std::mt19937 rng(seed ? seed : std::random_device{}());
        std::iota(perm.begin(), perm.begin() + TableSize, 0);
        std::shuffle(perm.begin(), perm.begin() + TableSize, rng);
        std::copy(perm.begin(), perm.begin() + TableSize, perm.begin() + TableSize);
    }

    static float fade(float t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static float grad(int hash, float x, float y)
    {
        int h = hash & 0b11;
        float u = (h < 2) ? x : y;
        float v = (h < 2) ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }

    float perlin(float x, float y)
    {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);

        float u = fade(x);
        float v = fade(y);

        int aa = perm[X] + Y;
        int ab = perm[X] + Y + 1;
        int ba = perm[X + 1] + Y;
        int bb = perm[X + 1] + Y + 1;

        float gradAA = grad(perm[aa], x, y);
        float gradBA = grad(perm[ba], x - 1, y);
        float gradAB = grad(perm[ab], x, y - 1);
        float gradBB = grad(perm[bb], x - 1, y - 1);

        float lerpX1 = utils::lerp(u, gradAA, gradBA);
        float lerpX2 = utils::lerp(u, gradAB, gradBB);

        return utils::lerp(v, lerpX1, lerpX2);
    }

    float layered_perlin(float x, float y, float amplitude, float frequency,
                         int octaves, float persistence, float lacunarity)
    {
        float total = 0.0f;
        for (int i = 0; i < octaves; ++i)
        {
            total += perlin(x * frequency, y * frequency) * amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }
        return total;
    }

} // namespace Noise
