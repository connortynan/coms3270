#include <cstring>
#include <endian.h>
#include <vector>
#include <stdexcept>
#include <cstdint>

#include "dungeon.h"

#define DUNGEON_FILE_HEADER "RLG327-S2025"
#define DUNGEON_HEADER_LEN 12

void Dungeon::serialize(std::ostream &out, mapsize_t pc_x, mapsize_t pc_y) const
{
    // Header
    out.write(DUNGEON_FILE_HEADER, DUNGEON_HEADER_LEN);

    // Version
    uint32_t version = htobe32(0);
    out.write(reinterpret_cast<const char *>(&version), sizeof(version));

    // Placeholder for file size
    std::streampos size_pos = out.tellp();
    uint32_t dummy_size = 0;
    out.write(reinterpret_cast<const char *>(&dummy_size), sizeof(dummy_size));

    // PC position (placeholders, must be filled when calling)
    out.write(reinterpret_cast<const char *>(&pc_x), 1);
    out.write(reinterpret_cast<const char *>(&pc_y), 1);

    // Hardness matrix and stair info
    std::vector<uint16_t> up_stairs;
    std::vector<uint16_t> down_stairs;

    for (mapsize_t y = 0; y < height; ++y)
        for (mapsize_t x = 0; x < width; ++x)
        {
            cell_type_t type = type_grid.at(x, y);
            if (type == CELL_STAIR_UP)
                up_stairs.push_back((x << 8) | y);
            else if (type == CELL_STAIR_DOWN)
                down_stairs.push_back((x << 8) | y);

            cell_hardness_t hardness = hardness_grid.at(x, y);
            out.write(reinterpret_cast<const char *>(&hardness), sizeof(cell_hardness_t));
        }

    // Rooms
    uint16_t num_rooms = htobe16(rooms.size());
    out.write(reinterpret_cast<const char *>(&num_rooms), sizeof(num_rooms));

    for (const auto &room : rooms)
    {
        char room_x = room.center_x - room.width / 2;
        char room_y = room.center_y - room.height / 2;
        char room_w = room.width;
        char room_h = room.height;

        out.write(&room_x, 1);
        out.write(&room_y, 1);
        out.write(&room_w, 1);
        out.write(&room_h, 1);
    }

    // Up stairs
    uint16_t ups = htobe16(up_stairs.size());
    out.write(reinterpret_cast<const char *>(&ups), sizeof(ups));
    for (uint16_t val : up_stairs)
        out.write(reinterpret_cast<const char *>(&val), sizeof(val));

    // Down stairs
    uint16_t downs = htobe16(down_stairs.size());
    out.write(reinterpret_cast<const char *>(&downs), sizeof(downs));
    for (uint16_t val : down_stairs)
        out.write(reinterpret_cast<const char *>(&val), sizeof(val));

    // Patch in file size
    std::streampos end_pos = out.tellp();
    uint32_t size = htobe32(static_cast<uint32_t>(end_pos));
    out.seekp(size_pos);
    out.write(reinterpret_cast<const char *>(&size), sizeof(size));
    out.seekp(end_pos);
}

Dungeon Dungeon::deserialize(std::istream &in, mapsize_t &pc_x, mapsize_t &pc_y)
{
    char marker[DUNGEON_HEADER_LEN + 1] = {};
    in.read(marker, DUNGEON_HEADER_LEN);
    if (std::strcmp(marker, DUNGEON_FILE_HEADER) != 0)
        throw std::runtime_error("Invalid file header");

    uint32_t version, file_size;
    in.read(reinterpret_cast<char *>(&version), sizeof(version));
    version = be32toh(version);
    in.read(reinterpret_cast<char *>(&file_size), sizeof(file_size));
    file_size = be32toh(file_size);

    uint8_t px, py;
    in.read(reinterpret_cast<char *>(&px), 1);
    in.read(reinterpret_cast<char *>(&py), 1);
    pc_x = px;
    pc_y = py;

    Dungeon dungeon(80, 21);

    for (mapsize_t y = 0; y < dungeon.height; ++y)
        for (mapsize_t x = 0; x < dungeon.width; ++x)
        {
            cell_hardness_t h;
            in.read(reinterpret_cast<char *>(&h), sizeof(h));
            dungeon.hardness_grid.at(x, y) = h;
            dungeon.type_grid.at(x, y) = (h > 0) ? CELL_ROCK : CELL_CORRIDOR;
        }

    uint16_t num_rooms_be;
    in.read(reinterpret_cast<char *>(&num_rooms_be), sizeof(num_rooms_be));
    uint16_t num_rooms = be16toh(num_rooms_be);

    dungeon.rooms.resize(num_rooms);
    for (uint16_t i = 0; i < num_rooms; ++i)
    {
        uint8_t room_x, room_y, room_w, room_h;
        in.read(reinterpret_cast<char *>(&room_x), 1);
        in.read(reinterpret_cast<char *>(&room_y), 1);
        in.read(reinterpret_cast<char *>(&room_w), 1);
        in.read(reinterpret_cast<char *>(&room_h), 1);

        RoomData &room = dungeon.rooms[i];
        room.center_x = room_x + room_w / 2;
        room.center_y = room_y + room_h / 2;
        room.width = room_w;
        room.height = room_h;

        for (mapsize_t y = room_y; y < room_y + room_h; ++y)
            for (mapsize_t x = room_x; x < room_x + room_w; ++x)
                dungeon.type_grid.at(x, y) = CELL_ROOM;
    }

    auto read_stairs = [&](cell_type_t stair_type)
    {
        uint16_t count_be;
        in.read(reinterpret_cast<char *>(&count_be), sizeof(count_be));
        uint16_t count = be16toh(count_be);

        for (uint16_t i = 0; i < count; ++i)
        {
            uint16_t pos;
            in.read(reinterpret_cast<char *>(&pos), sizeof(pos));
            uint8_t x = (pos >> 8) & 0xFF;
            uint8_t y = pos & 0xFF;
            dungeon.type_grid.at(x, y) = stair_type;
        }
    };

    read_stairs(CELL_STAIR_UP);
    read_stairs(CELL_STAIR_DOWN);

    return dungeon;
}
