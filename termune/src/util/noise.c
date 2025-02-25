#include "noise.h"
#include "util/generic_utils.h"
#include <math.h>
#include <string.h>

/// Noise permutation table used for seed-dependent noise generation.
static int noise_permutation[NOISE_PERMUTATION_TABLE_LEN * 2];

void noise_generate_permutation()
{
    for (int i = 0; i < NOISE_PERMUTATION_TABLE_LEN; i++)
    {
        noise_permutation[i] = i;
    }

    util_shuffle(noise_permutation, NOISE_PERMUTATION_TABLE_LEN, sizeof(*noise_permutation));

    memcpy(&noise_permutation[NOISE_PERMUTATION_TABLE_LEN], noise_permutation, NOISE_PERMUTATION_TABLE_LEN * sizeof(*noise_permutation));
}

static inline float noise_fade(const float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static inline float noise_grad(int hash, float x, float y)
{
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float noise_perlin(float x, float y)
{
    int X = ((int)floor(x)) % NOISE_PERMUTATION_TABLE_LEN;
    if (X < 0)
        X += NOISE_PERMUTATION_TABLE_LEN;

    int Y = ((int)floor(y)) % NOISE_PERMUTATION_TABLE_LEN;
    if (Y < 0)
        Y += NOISE_PERMUTATION_TABLE_LEN;

    x -= floorf(x);
    y -= floorf(y);

    float u = noise_fade(x);
    float v = noise_fade(y);

    int hash;
    hash = noise_permutation[(noise_permutation[X + 1] + Y + 1) % NOISE_PERMUTATION_TABLE_LEN];
    const float t00 = noise_grad(hash, x, y);

    hash = noise_permutation[(noise_permutation[X + 1] + Y) % NOISE_PERMUTATION_TABLE_LEN];
    const float t01 = noise_grad(hash, x, y - 1.f);

    hash = noise_permutation[(noise_permutation[X] + Y + 1) % NOISE_PERMUTATION_TABLE_LEN];
    const float t10 = noise_grad(hash, x - 1.f, y);

    hash = noise_permutation[(noise_permutation[X] + Y) % NOISE_PERMUTATION_TABLE_LEN];
    const float t11 = noise_grad(hash, x - 1.f, y - 1.f);

    return util_lerp(u, util_lerp(v, t00, t01), util_lerp(v, t10, t11));
}

float layered_noise_perlin(
    float x, float y,
    float amplitude, float frequency,
    int octaves, float persistence, float lacunarity)
{
    float result = 0.f;
    for (int i = 0; i < octaves; i++)
    {
        result += amplitude * noise_perlin(x * frequency, y * frequency);
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return result;
}
