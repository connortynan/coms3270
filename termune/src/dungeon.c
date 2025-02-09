#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "dungeon.h"
#include "util/vector.h"

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

int dungeon_serialize(const dungeon_data *dungeon, FILE *file)
{
    if (!dungeon || !file)
        return 1;

    uint8_t x, y;
    uint16_t i;
    vector *up_stairs = vector_init(0, sizeof(uint16_t));
    vector *down_stairs = vector_init(0, sizeof(uint16_t));

    if (!up_stairs || !down_stairs)
        return 1;

    // marker
    const char *marker = "RLG327-S2025";
    fwrite(marker, sizeof(*marker), 12, file);

    // version
    uint32_t version = htobe32(0);
    fwrite(&version, sizeof(version), 1, file);

    long file_pos = ftell(file);

    // file size (put dummy number for now, we'll update it later)
    uint32_t file_size = 0;
    fwrite(&file_size, sizeof(file_size), 1, file); // Dummy size for now

    // hardness matrix (also gather stair info)
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            if (dungeon->cells[x][y].type & (CELL_STAIR_UP | CELL_STAIR_DOWN))
            {
                uint8_t pos[2] = {x, y};
                vector_push_back(dungeon->cells[x][y].type == CELL_STAIR_UP ? up_stairs : down_stairs, pos);
            }

            fwrite(&dungeon->cells[x][y].hardness, sizeof(char), 1, file);
        }
    }

    // rooms: count
    uint16_t num_rooms = htobe16(dungeon->num_rooms);
    fwrite(&num_rooms, sizeof(uint16_t), 1, file);

    // rooms: info
    char room_x, room_y, room_w, room_h;
    for (i = 0; i < dungeon->num_rooms; i++)
    {
        room_x = dungeon->rooms[i].center_x;
        room_y = dungeon->rooms[i].center_y;
        room_w = dungeon->rooms[i].width;
        room_h = dungeon->rooms[i].height;

        // move center coordinate to top left corner
        room_x -= room_w / 2;
        room_y -= room_h / 2;

        fwrite(&room_x, sizeof(char), 1, file);
        fwrite(&room_y, sizeof(char), 1, file);
        fwrite(&room_w, sizeof(char), 1, file);
        fwrite(&room_h, sizeof(char), 1, file);
    }

    // up stairs: count
    uint16_t num_up_stairs = htobe16(up_stairs->size);
    fwrite(&num_up_stairs, sizeof(uint16_t), 1, file);

    // up stairs: info
    fwrite(up_stairs->_elems, sizeof(uint16_t), up_stairs->size, file);

    // down stairs: count
    uint16_t num_down_stairs = htobe16(down_stairs->size);
    fwrite(&num_down_stairs, sizeof(uint16_t), 1, file);

    // down stairs: info
    fwrite(down_stairs->_elems, sizeof(uint16_t), down_stairs->size, file);

    long end_pos = ftell(file);

    fseek(file, file_pos, SEEK_SET);

    file_size = htobe32(end_pos);
    fwrite(&file_size, sizeof(file_size), 1, file);

    fseek(file, end_pos, SEEK_SET);

    vector_destroy(up_stairs);
    vector_destroy(down_stairs);

    return 0;
}

int dungeon_deserialize(dungeon_data *dungeon, FILE *file)
{
    if (!dungeon || !file)
        return 1;

    char marker[12];
    fread(marker, sizeof(char), 12, file);

    return 1;
}
