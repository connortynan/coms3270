#pragma once

#include <vector>
#include <iostream>

#include "types.hpp"
#include "util/grid.hpp"

class Dungeon
{
public:
    enum cell_type_t
    {
        CELL_ROCK,
        CELL_ROOM,
        CELL_CORRIDOR,
        CELL_STAIR_UP,
        CELL_STAIR_DOWN,
    };
    static constexpr char char_map(cell_type_t type)
    {
        switch (type)
        {
        case CELL_ROCK:
            return ' ';
        case CELL_ROOM:
            return '.';
        case CELL_CORRIDOR:
            return '#';
        case CELL_STAIR_UP:
            return '<';
        case CELL_STAIR_DOWN:
            return '>';
        }
        return '?';
    }

    using cell_hardness_t = unsigned char;

    struct RoomData
    {
        mapsize_t center_x;
        mapsize_t center_y;
        mapsize_t width;
        mapsize_t height;
    };

    Dungeon(mapsize_t width, mapsize_t height)
        : width(width), height(height),
          type_grid(width, height, CELL_ROCK),
          hardness_grid(width, height, 0) {}

    void serialize(std::ostream &out, mapsize_t pc_x, mapsize_t pc_y) const;
    static Dungeon deserialize(std::istream &in, mapsize_t &pc_x, mapsize_t &pc_y);

    bool in_bounds(mapsize_t x, mapsize_t y) const
    {
        return (x >= 0) && (x < width) && (y >= 0) && (y < height);
    }

    // use parallel cells for data access optimizations, but allow returning all cell data for a given coordinate
    struct dungeon_cell_t
    {
        cell_type_t type;
        cell_hardness_t hardness;
    };

    dungeon_cell_t at(int x, int y) const
    {
        return dungeon_cell_t{
            .type = type_grid.at(x, y),
            .hardness = hardness_grid.at(x, y)};
    }

public:
    mapsize_t width;
    mapsize_t height;
    Grid<cell_type_t> type_grid;
    Grid<cell_hardness_t> hardness_grid;
    std::vector<RoomData> rooms;

public:
    class Generator
    {
    public:
        struct Parameters
        {
            mapsize_t min_room_width;
            mapsize_t max_room_width;

            mapsize_t min_room_height;
            mapsize_t max_room_height;

            mapsize_t min_num_rooms;
            mapsize_t max_num_rooms;

            mapsize_t min_num_stairs;
            mapsize_t max_num_stairs;

            cell_hardness_t min_rock_hardness;
            cell_hardness_t max_rock_hardness;

            unsigned char rock_hardness_smoothness;
            float rock_hardness_noise_amount;
        };

        static void generate_dungeon(Dungeon &dungeon, const Parameters &params, int seed);
    };
};