#pragma once

#include <math.h>
#include <stdint.h>
#include <string.h>
#include "util/util.h"

#define NOISE_PERMUTATION_TABLE_LEN 1024

static int noise_permutation[NOISE_PERMUTATION_TABLE_LEN * 2];

static inline void noise_generate_permutation()
{
    int i;
    for (i = 0; i < NOISE_PERMUTATION_TABLE_LEN; i++)
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

static inline void noise_hashed_vector(int v, float *x, float *y)
{
    *x = (v & 0b01) ? 1.f : -1.f;
    *y = (v & 0b10) ? 1.f : -1.f;
}

static inline float noise_perlin(float x, float y)
{
    // Take x and y modulo the length of the permutation table (dealing with negatives)
    int X = (int)fmod(x, NOISE_PERMUTATION_TABLE_LEN);
    X = (X + NOISE_PERMUTATION_TABLE_LEN) % NOISE_PERMUTATION_TABLE_LEN;
    int Y = (int)fmod(y, NOISE_PERMUTATION_TABLE_LEN);
    Y = (Y + NOISE_PERMUTATION_TABLE_LEN) % NOISE_PERMUTATION_TABLE_LEN;

    x -= floorf(x);
    y -= floorf(y);

    float u = noise_fade(x);
    float v = noise_fade(y);

    int hash;
    float vx, vy;

    hash = noise_permutation[noise_permutation[X + 1] + Y + 1];
    noise_hashed_vector(hash, &vx, &vy);
    const float t00 = util_dot(x - 1.f, y - 1.f, vx, vy);

    hash = noise_permutation[noise_permutation[X] + Y + 1];
    noise_hashed_vector(hash, &vx, &vy);
    const float t01 = util_dot(x, y - 1.f, vx, vy);

    hash = noise_permutation[noise_permutation[X + 1] + Y];
    noise_hashed_vector(hash, &vx, &vy);
    const float t10 = util_dot(x - 1.f, y, vx, vy);

    hash = noise_permutation[noise_permutation[X] + Y];
    noise_hashed_vector(hash, &vx, &vy);
    const float t11 = util_dot(x, y, vx, vy);

    return util_lerp(u,
                     util_lerp(v, t00, t01),
                     util_lerp(v, t10, t11));
}

static inline float layered_noise_perlin(
    float x, float y,
    float amplitude, float frequency,
    int octaves, float persistence, float lacunarity)
{
    float result = 0.f;
    int i;
    for (i = 0; i < octaves; i++)
    {
        result += amplitude * noise_perlin(x * frequency, y * frequency);
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return result;
}