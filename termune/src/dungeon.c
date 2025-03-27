#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "dungeon.h"
#include "util/vector.h"

#define DUNGEON_FILE_HEADER "RLG327-S2025"

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

void dungeon_generate_display_buffer(const dungeon_data *dungeon, char *result)
{
    int x, y;

    static const char char_map[] = {' ', '.', '#', '<', '>'};

    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            result[x + y * DUNGEON_WIDTH] = char_map[dungeon->cell_types[y][x]];
        }
    }
}

void dungeon_display(const dungeon_data *dungeon, const int display_border)
{
    uint8_t x, y;

    char output[DUNGEON_WIDTH * DUNGEON_HEIGHT];

    dungeon_generate_display_buffer(dungeon, output);

    if (display_border)
    {
        // Top border
        output[0] = '/';
        for (x = 1; x < DUNGEON_WIDTH - 1; x++)
            output[x] = '-';
        output[DUNGEON_WIDTH - 1] = '\\';

        // Side borders
        for (y = 1; y < DUNGEON_HEIGHT - 1; y++)
        {
            output[y * DUNGEON_WIDTH] = '|';
            output[(DUNGEON_WIDTH - 1) + y * DUNGEON_WIDTH] = '|';
        }

        // Bottom border
        output[(DUNGEON_HEIGHT - 1) * DUNGEON_WIDTH] = '\\';
        for (x = 1; x < DUNGEON_WIDTH - 1; x++)
            output[x + (DUNGEON_HEIGHT - 1) * DUNGEON_WIDTH] = '-';
        output[(DUNGEON_WIDTH - 1) + (DUNGEON_HEIGHT - 1) * DUNGEON_WIDTH] = '/';
    }

    // Display output
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
#ifdef DEBUG_DEV_FLAGS
            // Print hardness by color
            printf("\033[48;2;%d;127;127m%c\033[0m", dungeon->cell_hardness[y][x], output[x + y * DUNGEON_WIDTH]);
#else
            printf("%c", output[x + y * DUNGEON_WIDTH]);
#endif // DEBUG_DEV_FLAG
        }
        printf("\n");
    }
}

int dungeon_serialize(const dungeon_data *dungeon, FILE *file, uint8_t pc_x, uint8_t pc_y)
{
    if (!dungeon || !file)
        return 1;

    uint8_t x, y;
    uint16_t i, pos;
    vector *up_stairs = vector_init(0, sizeof(uint16_t));
    vector *down_stairs = vector_init(0, sizeof(uint16_t));

    if (!up_stairs || !down_stairs)
        return 1;

    // marker
    const char *marker = DUNGEON_FILE_HEADER;
    fwrite(marker, sizeof(char), 12, file);

    // version
    const uint32_t version = htobe32(0);
    fwrite(&version, sizeof(uint32_t), 1, file);

    // file size (put dummy number for now, we'll update it later)
    long file_pos = ftell(file);
    uint32_t file_size = 0;
    fwrite(&file_size, sizeof(uint32_t), 1, file);

    // pc coordinates
    fwrite(&pc_x, sizeof(uint8_t), 1, file);
    fwrite(&pc_y, sizeof(uint8_t), 1, file);

    // hardness matrix (also gather stair info)
    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            if (dungeon->cell_types[y][x] == CELL_STAIR_UP)
            {
                pos = ((uint16_t)x << 8) | y;
                vector_push_back(up_stairs, &pos);
            }
            else if (dungeon->cell_types[y][x] == CELL_STAIR_DOWN)
            {
                pos = ((uint16_t)x << 8) | y;
                vector_push_back(down_stairs, &pos);
            }

            fwrite(&dungeon->cell_types[y][x], sizeof(uint8_t), 1, file);
        }
    }

    // rooms: count
    const int16_t num_rooms = htobe16(dungeon->num_rooms);
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
    const uint16_t num_up_stairs = htobe16(up_stairs->size);
    fwrite(&num_up_stairs, sizeof(uint16_t), 1, file);

    // up stairs: info
    fwrite(up_stairs->_elems, sizeof(uint16_t), up_stairs->size, file);

    // down stairs: count
    const uint16_t num_down_stairs = htobe16(down_stairs->size);
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

int dungeon_deserialize(dungeon_data *dungeon, FILE *file, uint8_t *pc_x, uint8_t *pc_y)
{
    if (!dungeon || !file)
        return 1;

    dungeon->north = dungeon->east = dungeon->south = dungeon->west = dungeon->up = dungeon->down = NULL;

    uint8_t x, y;
    uint16_t i;

    // marker
    char marker[13];
    fread(marker, sizeof(char), 12, file);
    marker[12] = 0;
    if (strcmp(DUNGEON_FILE_HEADER, marker))
    {
        fprintf(stderr, "File read error, unknown file marker\n");
        return 1;
    }

    // version
    uint32_t version;
    fread(&version, sizeof(uint32_t), 1, file);
    version = be32toh(version);

    // file size
    uint32_t file_size;
    fread(&file_size, sizeof(uint32_t), 1, file);
    file_size = be32toh(file_size);

    // pc coordinates
    fread(pc_x, sizeof(uint8_t), 1, file);
    fread(pc_y, sizeof(uint8_t), 1, file);

    // hardness
    fread(&dungeon->cell_hardness[0][0], sizeof(uint8_t), DUNGEON_WIDTH * DUNGEON_HEIGHT, file);

    for (y = 0; y < DUNGEON_HEIGHT; y++)
    {
        for (x = 0; x < DUNGEON_WIDTH; x++)
        {
            // Fill all >0 to rock, and all =0 to corridors (other 0-hardness cells with overwrite this)
            dungeon->cell_types[y][x] = (dungeon->cell_hardness[y][x] > 0) ? CELL_ROCK : CELL_CORRIDOR;
        }
    }

    // rooms
    uint16_t num_rooms;
    fread(&num_rooms, sizeof(uint16_t), 1, file);
    num_rooms = be16toh(num_rooms);

    dungeon->num_rooms = num_rooms;
    dungeon->rooms = (dungeon_room_data *)malloc(dungeon->num_rooms * sizeof(*dungeon->rooms));

    uint8_t room_x, room_y, room_w, room_h;
    for (i = 0; i < num_rooms; i++)
    {
        fread(&room_x, sizeof(uint8_t), 1, file);
        fread(&room_y, sizeof(uint8_t), 1, file);
        fread(&room_w, sizeof(uint8_t), 1, file);
        fread(&room_h, sizeof(uint8_t), 1, file);

        dungeon->rooms[i].center_x = room_x + room_w / 2;
        dungeon->rooms[i].center_y = room_y + room_h / 2;
        dungeon->rooms[i].width = room_w;
        dungeon->rooms[i].height = room_h;

        for (x = room_x; x < room_x + room_w; x++)
        {
            for (y = room_y; y < room_y + room_h; y++)
            {
                dungeon->cell_types[y][x] = CELL_ROOM;
            }
        }
    }

    uint16_t pos;
    uint8_t stair_x, stair_y;

    // up stairs
    uint16_t num_up_stairs;
    fread(&num_up_stairs, sizeof(uint16_t), 1, file);
    num_up_stairs = be16toh(num_up_stairs);

    for (i = 0; i < num_up_stairs; i++)
    {
        fread(&pos, sizeof(uint16_t), 1, file);
        stair_x = (pos >> 8) & 0xFF;
        stair_y = (pos) & 0xFF;

        dungeon->cell_types[stair_y][stair_x] = CELL_STAIR_UP;
    }

    // down stairs
    uint16_t num_down_stairs;
    fread(&num_down_stairs, sizeof(uint16_t), 1, file);
    num_down_stairs = be16toh(num_down_stairs);

    for (i = 0; i < num_down_stairs; i++)
    {
        fread(&pos, sizeof(uint16_t), 1, file);
        stair_x = (pos >> 8) & 0xFF;
        stair_y = (pos) & 0xFF;

        dungeon->cell_types[stair_y][stair_x] = CELL_STAIR_DOWN;
    }

    return 0;
}
