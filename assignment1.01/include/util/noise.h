#pragma once

#include <math.h>
#include <stdint.h>

#include "util/util_math.h"

// Gradient values for the permutation table
static const int noise_permutation[] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};

/**
 * @brief Computes the fade curve for smooth interpolation in Perlin noise.
 *
 * This function implements a quintic polynomial curve (6t^5 - 15t^4 + 10t^3)
 * that smooths the transitions in Perlin noise by reducing sharp edges.
 *
 * @param t The input value, typically in the range [0, 1].
 * @return The smoothed output value.
 */
static float noise_fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}


/**
 * @brief Computes the gradient contribution from a corner in Perlin noise.
 *
 * The function takes a hashed value and coordinates to compute the dot product
 * between a gradient vector and the distance vector from the corner.
 *
 * @param hash Hash value used to select the gradient direction.
 * @param x X-coordinate of the point.
 * @param y Y-coordinate of the point.
 * @param z Z-coordinate of the point.
 * @return The dot product contribution from the corner.
 */
static float noise_grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

/**
 * @brief Computes Perlin noise for a 3D point.
 *
 * This function uses fade curves, gradients, and a permutation table to generate
 * smooth noise values at the specified point in 3D space.
 *
 * @param x X-coordinate of the point in space.
 * @param y Y-coordinate of the point in space.
 * @param z Z-coordinate of the point in space.
 * @return The computed Perlin noise value.
 */
float noise_perlin(float x, float y, float z) {
    // Find unit cube containing the point
    int X = (int)floorf(x) & 255;
    int Y = (int)floorf(y) & 255;
    int Z = (int)floorf(z) & 255;

    // Relative coordinates within the cube
    x -= floorf(x);
    y -= floorf(y);
    z -= floorf(z);

    // Compute fade curves
    float u = noise_fade(x);
    float v = noise_fade(y);
    float w = noise_fade(z);

    // Hash coordinates of the cube corners
    int A = noise_permutation[X] + Y;
    int AA = noise_permutation[A] + Z;
    int AB = noise_permutation[A + 1] + Z;
    int B = noise_permutation[X + 1] + Y;
    int BA = noise_permutation[B] + Z;
    int BB = noise_permutation[B + 1] + Z;

    // Add blended results from the corners
    float result = noise_lerp(
        w,
        noise_lerp(v,
                   math_lerp(u, noise_grad(noise_permutation[AA], x, y, z),
                             noise_grad(noise_permutation[BA], x - 1, y, z)),
                   math_lerp(u, noise_grad(noise_permutation[AB], x, y - 1, z),
                             noise_grad(noise_permutation[BB], x - 1, y - 1, z))),
        noise_lerp(v,
                   math_lerp(u, noise_grad(noise_permutation[AA + 1], x, y, z - 1),
                             noise_grad(noise_permutation[BA + 1], x - 1, y, z - 1)),
                   math_lerp(u, noise_grad(noise_permutation[AB + 1], x, y - 1, z - 1),
                             noise_grad(noise_permutation[BB + 1], x - 1, y - 1, z - 1))));

    return result;
}
