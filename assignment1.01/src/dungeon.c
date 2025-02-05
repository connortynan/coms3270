#include <stdio.h>
#include <string.h>

#include "dungeon.h"

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
#ifdef TERMUNE_DEV_DEBUG
            if (dungeon->cells[x][y].type == DEBUG_SHOW_HARDNESS)
            {
                // Copy hex representation to temporary string, and pull first character to display
                char displayed_hardness[8];
                sprintf(displayed_hardness, "%x", dungeon->cells[x][y].hardness % 16);
                output[x][y] = displayed_hardness[0];
            }
            else
#endif // TERMUNE_DEV_DEBUG
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
            printf("%c", output[x][y]);
        }
        printf("\n");
    }
}