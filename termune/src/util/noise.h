#pragma once

namespace Noise
{
    void generate_permutation(unsigned int seed);

    float perlin(float x, float y);

    float layered_perlin(float x, float y,
                         float amplitude,
                         float frequency,
                         int octaves,
                         float persistence,
                         float lacunarity);

} // namespace Noise
