#include "dungeon.h"

#include <cmath>
#include <random>
#include <limits>

#include "util/noise.h"
#include "util/pathing.h"
#include "util/img_proc.h"

void Dungeon::Generator::generate_dungeon(Dungeon &dungeon, const Parameters &params, int seed = 0)
{
    constexpr std::size_t BUCKET_SIZE = 256;

    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);

    auto randint = [&](int lo, int hi)
    {
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    };

    auto randf = [&]()
    {
        return std::uniform_real_distribution<float>(0.f, 1.f)(rng);
    };

    // --- Clear map ---
    for (mapsize_t y = 0; y < dungeon.height; ++y)
        for (mapsize_t x = 0; x < dungeon.width; ++x)
            dungeon.type_grid.at(x, y) = CELL_ROCK;

    dungeon.rooms.clear();

    // --- Generate candidate rooms ---
    std::vector<RoomData> room_bucket;
    for (std::size_t i = 0; i < BUCKET_SIZE; ++i)
    {
        RoomData r;
        r.width = randint(params.min_room_width, params.max_room_width);
        r.height = randint(params.min_room_height, params.max_room_height);
        r.center_x = (r.width / 2 + 1) + randint(0, dungeon.width - r.width - 3);
        r.center_y = (r.height / 2 + 1) + randint(0, dungeon.height - r.height - 3);
        room_bucket.push_back(r);
    }

    const std::size_t target_rooms = randint(params.min_num_rooms, params.max_num_rooms);
    std::vector<size_t> placed_indices;
    std::size_t bucket_idx = 0;

    while (dungeon.rooms.size() < target_rooms)
    {
        if (bucket_idx >= room_bucket.size())
        {
            if (dungeon.rooms.size() >= params.min_num_rooms)
                break;

            if (dungeon.rooms.empty())
                return;

            bucket_idx = placed_indices.back() + 1;
            placed_indices.pop_back();
            dungeon.rooms.pop_back();
            continue;
        }

        const RoomData &room = room_bucket[bucket_idx];

        bool fits = true;
        int min_x = room.center_x - room.width / 2 - 1;
        int max_x = room.center_x + (room.width - 1) / 2 + 1;
        int min_y = room.center_y - room.height / 2 - 1;
        int max_y = room.center_y + (room.height - 1) / 2 + 1;

        if (min_x < 0 || max_x >= dungeon.width || min_y < 0 || max_y >= dungeon.height)
        {
            fits = false;
        }
        else
        {
            for (const auto &existing : dungeon.rooms)
            {
                int ex_min_x = existing.center_x - existing.width / 2;
                int ex_max_x = existing.center_x + (existing.width - 1) / 2;
                int ex_min_y = existing.center_y - existing.height / 2;
                int ex_max_y = existing.center_y + (existing.height - 1) / 2;

                if (max_x >= ex_min_x && min_x <= ex_max_x &&
                    max_y >= ex_min_y && min_y <= ex_max_y)
                {
                    fits = false;
                    break;
                }
            }
        }

        if (fits)
        {
            dungeon.rooms.push_back(room);
            placed_indices.push_back(bucket_idx);

            for (int dx = 0; dx < room.width; ++dx)
            {
                for (int dy = 0; dy < room.height; ++dy)
                {
                    mapsize_t x = room.center_x - room.width / 2 + dx;
                    mapsize_t y = room.center_y - room.height / 2 + dy;
                    dungeon.type_grid.at(x, y) = CELL_ROOM;
                    dungeon.hardness_grid.at(x, y) = 0;
                }
            }
        }

        ++bucket_idx;
    }

    // --- Assign rock hardnesses per room ---
    std::vector<cell_hardness_t> room_hardness(dungeon.rooms.size());

    for (size_t i = 0; i < dungeon.rooms.size(); ++i)
    {
        room_hardness[i] = randint(params.min_rock_hardness, params.max_rock_hardness);
    }

    for (mapsize_t y = 0; y < dungeon.height; ++y)
    {
        for (mapsize_t x = 0; x < dungeon.width; ++x)
        {
            size_t closest = 0;
            int min_dist_sq = std::numeric_limits<int>::max();
            for (size_t i = 0; i < dungeon.rooms.size(); ++i)
            {
                int dx = dungeon.rooms[i].center_x - x;
                int dy = dungeon.rooms[i].center_y - y;
                int dist_sq = dx * dx + dy * dy;
                if (dist_sq < min_dist_sq)
                {
                    closest = i;
                    min_dist_sq = dist_sq;
                }
            }
            dungeon.hardness_grid.at(x, y) = room_hardness[closest];
        }
    }

    for (int i = 0; i < params.rock_hardness_smoothness; ++i)
    {
        gaussian_blur(dungeon.hardness_grid, true);
    }

    if (params.rock_hardness_noise_amount > 0.f)
    {
        float x_off = randf() * 256.f;
        float y_off = randf() * 256.f;

        for (mapsize_t y = 0; y < dungeon.height; ++y)
        {
            for (mapsize_t x = 0; x < dungeon.width; ++x)
            {
                float noise = Noise::layered_perlin(x + x_off, y + y_off,
                                                    params.rock_hardness_noise_amount,
                                                    0.15f, 8, 0.5f, 2.f);
                int raw = dungeon.hardness_grid.at(x, y) + static_cast<int>(noise);
                dungeon.hardness_grid.at(x, y) = std::clamp(raw, 1, 254);
            }
        }
    }

    // --- Finalize hardness values ---
    for (mapsize_t y = 0; y < dungeon.height; ++y)
    {
        for (mapsize_t x = 0; x < dungeon.width; ++x)
        {

            if (x == 0 || y == 0 || x == dungeon.width - 1 || y == dungeon.height - 1)
                dungeon.hardness_grid.at(x, y) = 255;
            else if (dungeon.type_grid.at(x, y) != CELL_ROCK)
                dungeon.hardness_grid.at(x, y) = 0;
        }
    }

    // Corridor generation
    for (std::size_t i = 0; i < dungeon.rooms.size(); ++i)
    {
        const auto &start = dungeon.rooms[i];
        const auto &end = dungeon.rooms[(i + 1) % dungeon.rooms.size()];

        struct DungeonNode : public Pathing::Node
        {
            mapsize_t x, y;
            Grid<cell_hardness_t> *hardness_grid;
            mapsize_t goal_x, goal_y;

            DungeonNode(mapsize_t x, mapsize_t y, Grid<cell_hardness_t> *g, mapsize_t gx, mapsize_t gy)
                : x(x), y(y), hardness_grid(g), goal_x(gx), goal_y(gy) {}

            std::vector<std::size_t> get_neighbors() const override
            {
                std::vector<std::size_t> n;
                if (static_cast<std::size_t>(x) > 0)
                    n.push_back(y * hardness_grid->width() + (x - 1));
                if (static_cast<std::size_t>(x + 1) < hardness_grid->width())
                    n.push_back(y * hardness_grid->width() + (x + 1));
                if (static_cast<std::size_t>(y) > 0)
                    n.push_back((y - 1) * hardness_grid->width() + x);
                if (static_cast<std::size_t>(y + 1) < hardness_grid->height())
                    n.push_back((y + 1) * hardness_grid->width() + x);
                return n;
            }

            uint64_t movement_cost_to(const Node &neighbor) const override
            {
                auto &n = static_cast<const DungeonNode &>(neighbor);
                return hardness_grid->at(n.x, n.y);
            }

            bool is_goal() const override
            {
                return x == goal_x && y == goal_y;
            }
        };

        std::vector<std::unique_ptr<Pathing::Node>> nodes;
        for (mapsize_t y = 0; y < dungeon.height; ++y)
            for (mapsize_t x = 0; x < dungeon.width; ++x)
                nodes.emplace_back(std::make_unique<DungeonNode>(x, y,
                                                                 &dungeon.hardness_grid,
                                                                 end.center_x, end.center_y));

        size_t start_idx = start.center_y * dungeon.width + start.center_x;
        auto path = Pathing::solve(nodes, start_idx);

        for (size_t idx : path)
        {
            mapsize_t px = idx % dungeon.width;
            mapsize_t py = idx / dungeon.width;

            if (dungeon.type_grid.at(px, py) == CELL_ROCK)
            {
                dungeon.type_grid.at(px, py) = CELL_CORRIDOR;
                dungeon.hardness_grid.at(px, py) = 0;
            }
        }
    }

    // --- Place stairs ---
    int stair_dir = randint(0, 1);
    size_t num_stairs = randint(params.min_num_stairs, params.max_num_stairs);
    for (size_t i = 0; i < num_stairs; ++i)
    {
        mapsize_t x, y;
        do
        {
            x = randint(0, dungeon.width - 1);
            y = randint(0, dungeon.height - 1);
        } while (dungeon.type_grid.at(x, y) == CELL_ROCK);

        dungeon.type_grid.at(x, y) = stair_dir ? CELL_STAIR_DOWN : CELL_STAIR_UP;
        dungeon.hardness_grid.at(x, y) = 0;
        stair_dir ^= 1;
    }
}
