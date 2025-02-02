#include <stdio.h>

#include "dungeon.h"
#include "generator.h"
#include "util.h"

int main(int argc, char const *argv[])
{
    packed_bool_array bools;

    bool_array_reserve(&bools, 70);
    bool_array_fill(&bools, 1);
    bool_array_set(&bools, 70, 0);
    bool_array_set(&bools, 5, 0);

    for (size_t i = 0; i < bools._chunk_n; i++)
    {
        for (uint8_t bit = 0; bit < PACKED_BOOL_CHUNK_BITS; bit++)
        {
            printf("%c", ((bools._data[i] >> bit) & 1) ? '1' : '0');
        }
        printf("\n");
    }

    printf("%d: %c\n", 0, bool_array_get(&bools, 0) ? '1' : '0');
    printf("%d: %c\n", 70, bool_array_get(&bools, 70) ? '1' : '0');
    printf("%d: %c\n", 5, bool_array_get(&bools, 5) ? '1' : '0');

    bool_array_set(&bools, 5, 1);

    for (size_t i = 0; i < bools._chunk_n; i++)
    {
        for (uint8_t bit = 0; bit < PACKED_BOOL_CHUNK_BITS; bit++)
        {
            printf("%c", (bools._data[i] >> bit & 1) ? '1' : '0');
        }
        printf("\n");
    }

    printf("%d: %c\n", 0, bool_array_get(&bools, 0) ? '1' : '0');
    printf("%d: %c\n", 70, bool_array_get(&bools, 70) ? '1' : '0');
    printf("%d: %c\n", 5, bool_array_get(&bools, 5) ? '1' : '0');

    return 0;
}
