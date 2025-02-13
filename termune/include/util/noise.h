#pragma once

#include <math.h>
#include <stdint.h>
#include <string.h>
#include "util/util.h"

/// Length of the noise permutation table.
#define NOISE_PERMUTATION_TABLE_LEN 256

/// Noise permutation table used for seed-dependent noise generation.
static int noise_permutation[NOISE_PERMUTATION_TABLE_LEN * 2];

/**
 * @brief Generates a random permutation table for Perlin noise.
 *
 * This function initializes and shuffles the noise permutation table, then duplicates it
 * to avoid boundary issues when wrapping values.
 */
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

/**
 * @brief Smoothstep function used in Perlin noise interpolation.
 *
 * This function smooths a value `t` using the polynomial 6t^5 - 15t^4 + 10t^3 to
 * achieve a smooth transition between noise values.
 *
 * @param t Input value in the range [0, 1].
 * @return Smoothed value.
 */
static inline float noise_fade(const float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

/**
 * @brief Computes gradient vector dot product for Perlin noise.
 *
 * This function calculates the dot product between the gradient vector and
 * the input coordinate offset to create directional variations in the noise.
 *
 * @param hash Hash value used to determine gradient direction.
 * @param x X offset from grid point.
 * @param y Y offset from grid point.
 * @return Dot product of the gradient vector and the input coordinates.
 */
static inline float noise_grad(int hash, float x, float y)
{
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

/**
 * @brief Computes 2D Perlin noise at given coordinates.
 *
 * This function calculates a Perlin noise value based on an input position `(x, y)`.
 * It interpolates between noise values at the surrounding grid points.
 *
 * @param x X-coordinate.
 * @param y Y-coordinate.
 * @return Perlin noise value in the range [-1, 1].
 */
static inline float noise_perlin(float x, float y)
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

    return util_lerp(u,
                     util_lerp(v, t00, t01),
                     util_lerp(v, t10, t11));
}

/**
 * @brief Generates layered Perlin noise (Fractional Brownian Motion).
 *
 * This function creates multi-octave Perlin noise by summing noise layers at
 * increasing frequencies and decreasing amplitudes.
 *
 * @param x X-coordinate.
 * @param y Y-coordinate.
 * @param amplitude Initial amplitude of the noise.
 * @param frequency Initial frequency of the noise.
 * @param octaves Number of noise layers.
 * @param persistence Amplitude multiplier per octave (controls roughness).
 * @param lacunarity Frequency multiplier per octave (controls detail).
 * @return Layered Perlin noise value.
 */
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