#include <stdio.h>
#include <string.h>

#include "dungeon.h"

#ifdef DEBUG_DEV_FLAGS
#define DEBUG_GRADIENT_VALUES
#endif

int dungeon_destroy(dungeon_data *dungeon)
{
    if (dungeon->north)
        dungeon->north->south = NULL;
    if (dungeon->east)
        dungeon->east->west = NULL;
    if (dungeon->south)
        dungeon->south->north = NULL;
    if (dungeon->west)
        dungeon->west->east = NULL;
    if (dungeon->up)
        dungeon->up->down = NULL;
    if (dungeon->down)
        dungeon->down->up = NULL;

    free(dungeon->rooms);

    return 0;
}

void dungeon_display(const dungeon_data *dungeon, const int display_border)
{
    int x, y;

    static const char char_map[] = {' ', '.', '#', '>', '<'};

    char output[DUNGEON_WIDTH][DUNGEON_HEIGHT];

    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            output[x][y] = char_map[dungeon->cells[x][y].type];
        }
    }

    if (display_border)
    {
        // Top border
        output[0][0] = '/';
        for (x = 1; x < DUNGEON_WIDTH - 1; x++)
            output[x][0] = '-';
        output[DUNGEON_WIDTH - 1][0] = '\\';

        // Side borders
        for (y = 1; y < DUNGEON_HEIGHT - 1; y++)
        {
            output[0][y] = '|';
            output[DUNGEON_WIDTH - 1][y] = '|';
        }

        // Bottom border
        output[0][DUNGEON_HEIGHT - 1] = '\\';
        for (x = 1; x < DUNGEON_WIDTH - 1; x++)
            output[x][DUNGEON_HEIGHT - 1] = '-';
        output[DUNGEON_WIDTH - 1][DUNGEON_HEIGHT - 1] = '/';
    }

    // Display output
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {

#ifdef DEBUG_DEV_FLAGS
            // Print hardness by color
            printf("\033[48;2;%d;127;127m%c\033[0m", dungeon->cells[x][y].hardness, output[x][y]);
#else
            printf("%c", output[x][y]);
#endif // DEBUG_DEV_FLAG
        }
        printf("\n");
    }
}