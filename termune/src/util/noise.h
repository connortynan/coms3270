#pragma once

#include <array>
#include <cstdint>

namespace Noise
{
    constexpr int TableSize = 512;

    // Externally-defined permutation table
    extern std::array<uint8_t, TableSize * 2> perm;

    /**
     * @brief Initialize the permutation table using a random or fixed seed.
     * @param seed A 32-bit seed (0 = random seed)
     */
    void generate_permutation(uint32_t seed = 0);

    /**
     * @brief Generate 2D Perlin noise value at coordinate (x, y)
     */
    float perlin(float x, float y);

    /**
     * @brief Generate layered Perlin noise with multiple octaves
     */
    float layered_perlin(
        float x, float y,
        float amplitude,
        float frequency,
        int octaves,
        float persistence,
        float lacunarity);

} // namespace Noise
