#include "noise.h"
#include <stdlib.h>
#include <math.h>

static uint32_t perm[NOISE_PERMUTATION_TABLE_LEN];

static float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float t, float a, float b)
{
    return a + t * (b - a);
}

static float grad(int hash, float x, float y)
{
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

void noise_generate_permutation()
{
    for (int i = 0; i < NOISE_PERMUTATION_TABLE_LEN; i++)
    {
        perm[i] = i;
    }

    for (int i = 0; i < NOISE_PERMUTATION_TABLE_LEN; i++)
    {
        int j = rand() % NOISE_PERMUTATION_TABLE_LEN;
        uint32_t temp = perm[i];
        perm[i] = perm[j];
        perm[j] = temp;
    }
}

float noise_perlin(float x, float y)
{
    int X = (int)floorf(x) & 255;
    int Y = (int)floorf(y) & 255;

    x -= floorf(x);
    y -= floorf(y);

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

    float lerpX1 = lerp(u, gradAA, gradBA);
    float lerpX2 = lerp(u, gradAB, gradBB);

    return lerp(v, lerpX1, lerpX2);
}

float layered_noise_perlin(float x, float y, float amplitude, float frequency, int octaves, float persistence, float lacunarity)
{
    float total = 0.0f;

    for (int i = 0; i < octaves; i++)
    {
        total += noise_perlin(x * frequency, y * frequency) * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total;
}
