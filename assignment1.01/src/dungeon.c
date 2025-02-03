#include <stdio.h>

#include "dungeon.h"

void dungeon_display(const dungeon_data *dungeon, const int display_border)
{
    int x, y, i;

    static const char char_map[] = {' ', '.', '#', '>', '<'};

    if (display_border)
    {
        // Top border
        printf("╔");
        for (i = 0; i < DUNGEON_WIDTH - 2; i++)
            printf("═");
        printf("╗\n");

        // Main display (with side borders)
        for (y = 1; y < DUNGEON_HEIGHT - 1; y++)
        {
            printf("║");
            for (x = 1; x < DUNGEON_WIDTH - 1; x++)
                printf("%c", char_map[dungeon->cells[x][y].type]);

            printf("║\n");
        }

        // Bottom border
        printf("╚");
        for (i = 0; i < DUNGEON_WIDTH - 2; i++)
            printf("═");
        printf("╝\n");
    }
    else
    {
        for (y = 0; y < DUNGEON_HEIGHT; y++)
        {
            for (x = 0; x < DUNGEON_WIDTH; x++)
            {
                printf("%c", char_map[dungeon->cells[x][y].type]);
            }
            printf("\n");
        }
    }
}