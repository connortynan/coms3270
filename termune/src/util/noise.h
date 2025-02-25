#pragma once

#include <stdint.h>

/// Length of the noise permutation table.
#define NOISE_PERMUTATION_TABLE_LEN 256

/**
 * @brief Generates a random permutation table for Perlin noise.
 */
void noise_generate_permutation();

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
float noise_perlin(float x, float y);

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
float layered_noise_perlin(
    float x, float y,
    float amplitude, float frequency,
    int octaves, float persistence, float lacunarity);
