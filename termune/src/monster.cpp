#include "entity.hpp"

#include "game_context.hpp"
#include "util/pathing.hpp"
#include "util/shadowcast.hpp"
#include "dungeon.hpp"

class DistanceNode : public Pathing::Node
{
public:
    mapsize_t x, y;
    Grid<Dungeon::cell_hardness_t> *weights;
    mapsize_t w, h;

    DistanceNode(mapsize_t x, mapsize_t y, Grid<Dungeon::cell_hardness_t> *map)
        : x(x), y(y), weights(map), w(map->width()), h(map->height()) {}

    std::vector<std::size_t> get_neighbors() const override
    {
        std::vector<std::size_t> neighbors;
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                if (dx == 0 && dy == 0)
                    continue;
                mapsize_t nx = x + dx;
                mapsize_t ny = y + dy;
                if (nx < w && ny < h && weights->at(nx, ny) < 255)
                    neighbors.push_back(ny * w + nx);
            }
        }
        return neighbors;
    }

    unsigned long movement_cost_to(const Node &n) const override
    {
        const auto &other = static_cast<const DistanceNode &>(n);
        return 1 + weights->at(other.x, other.y) / 85;
    }

    bool is_goal() const override { return false; }
};

void Monster::update_global_maps(GameContext &game)
{
    // line-of-sight map
    Grid<unsigned char> solid_map(game.dungeon.width, game.dungeon.height, false);

    for (mapsize_t y = 0; y < game.dungeon.height; y++)
        for (mapsize_t x = 0; x < game.dungeon.width; x++)
        {
            solid_map(x, y) = (game.dungeon.type_grid(x, y) == Dungeon::CELL_ROCK);
        }

    game.monster_line_of_sight_map = ShadowCast::solve_lightmap(solid_map, game.player->x, game.player->y, 255);

    // distance grids
    Grid<Dungeon::cell_hardness_t> tunneling_weights(game.dungeon.width, game.dungeon.height, 0);
    Grid<Dungeon::cell_hardness_t> non_weights(game.dungeon.width, game.dungeon.height, 0);

    for (mapsize_t y = 0; y < game.dungeon.height; y++)
        for (mapsize_t x = 0; x < game.dungeon.width; x++)
        {
            if (game.dungeon.type_grid(x, y) == Dungeon::CELL_ROCK)
                non_weights.at(x, y) = 255;
            tunneling_weights.at(x, y) = game.dungeon.hardness_grid(x, y);
        }

    auto solve = [&](Grid<uint32_t> &out, Grid<uint8_t> &weights)
    {
        std::vector<std::unique_ptr<Pathing::Node>> nodes;
        for (mapsize_t y = 0; y < game.dungeon.height; y++)
            for (mapsize_t x = 0; x < game.dungeon.width; x++)
            {
                nodes.push_back(std::make_unique<DistanceNode>(x, y, &weights));
            }

        std::size_t start_idx = game.player->x + game.player->y * game.dungeon.width;
        Pathing::solve(nodes, start_idx);

        for (std::size_t i = 0; i < nodes.size(); i++)
        {
            mapsize_t x = i % game.dungeon.width;
            mapsize_t y = i / game.dungeon.width;
            out(x, y) = static_cast<uint32_t>(nodes[i]->cost);
        }
    };

    solve(game.monster_tunneling_map, tunneling_weights);
    solve(game.monster_nontunneling_map, non_weights);
}

void Monster::get_desired_move(int &dx, int &dy, GameContext &g) const
{
    dx = 0;
    dy = 0;

    // Erratic: 50% chance to move randomly
    if (erratic && (rand() % 2))
    {
        dx = (rand() % 3) - 1;
        dy = (rand() % 3) - 1;
        return;
    }

    // If we have LOS (e.g. just saw the player), follow directly
    if (has_line_of_sight)
    {
        if (target_x != EMPTY_TARGET && target_y != EMPTY_TARGET)
        {
            int dx_raw = static_cast<int>(target_x) - static_cast<int>(x);
            int dy_raw = static_cast<int>(target_y) - static_cast<int>(y);

            dx = (dx_raw == 0) ? 0 : (dx_raw > 0 ? 1 : -1);
            dy = (dy_raw == 0) ? 0 : (dy_raw > 0 ? 1 : -1);
        }
        return;
    }

    // Intelligent + Telepathic: use Dijkstra distance maps
    if (intelligent && telepathic)
    {
        const auto &dist_map = tunneling ? g.monster_tunneling_map : g.monster_nontunneling_map;

        mapsize_t best_x = x;
        mapsize_t best_y = y;
        uint32_t min_cost = dist_map(x, y);

        // Randomize starting direction
        int start = rand() % 8;
        const int offsets[8][2] = {
            {-1, -1},
            {0, -1},
            {1, -1},
            {-1, 0},
            {1, 0},
            {-1, 1},
            {0, 1},
            {1, 1},
        };

        for (int i = 0; i < 8; ++i)
        {
            int idx = (start + i) % 8;
            mapsize_t nx = x + offsets[idx][0];
            mapsize_t ny = y + offsets[idx][1];

            if (nx >= dist_map.width() || ny >= dist_map.height())
                continue;

            uint32_t cost = dist_map(nx, ny);
            if (cost < min_cost)
            {
                min_cost = cost;
                best_x = nx;
                best_y = ny;
            }
        }

        dx = static_cast<int>(best_x) - static_cast<int>(x);
        dy = static_cast<int>(best_y) - static_cast<int>(y);
        return;
    }

    // Intelligent + non-telepathic: move toward last seen player position
    if (intelligent && !telepathic)
    {
        if (target_x == EMPTY_TARGET || target_y == EMPTY_TARGET)
        {
            dx = 0;
            dy = 0;
            return;
        }

        int dx_raw = static_cast<int>(target_x) - static_cast<int>(x);
        int dy_raw = static_cast<int>(target_y) - static_cast<int>(y);

        dx = (dx_raw == 0) ? 0 : (dx_raw > 0 ? 1 : -1);
        dy = (dy_raw == 0) ? 0 : (dy_raw > 0 ? 1 : -1);
        return;
    }

    // Dumb telepathic: always follow blindly in a straight line
    if (!intelligent && telepathic)
    {
        int dx_raw = static_cast<int>(target_x) - static_cast<int>(x);
        int dy_raw = static_cast<int>(target_y) - static_cast<int>(y);

        dx = (dx_raw == 0) ? 0 : (dx_raw > 0 ? 1 : -1);
        dy = (dy_raw == 0) ? 0 : (dy_raw > 0 ? 1 : -1);
        return;
    }

    // Dumb + non-telepathic + no LOS = stand still
    dx = 0;
    dy = 0;
}

bool Monster::move(int dx, int dy, GameContext &g, bool force)
{
    // Update LOS / target
    has_line_of_sight = g.monster_line_of_sight_map(x, y);
    if (telepathic || has_line_of_sight)
    {
        target_x = g.player->x;
        target_y = g.player->y;
    }

    // Decide move
    get_desired_move(dx, dy, g);

    mapsize_t target_x = x + dx;
    mapsize_t target_y = y + dy;

    if (!g.dungeon.in_bounds(target_x, target_y) && dx == 0 && dy == 0)
        return false;

    auto target_cell = g.dungeon.type_grid(target_x, target_y);
    auto hardness = g.dungeon.hardness_grid(target_x, target_y);

    // Check for player/entity at destination
    auto entity = g.entity_at(target_x, target_y);
    if (entity && entity->id == ENTITY_PLAYER)
    {
        g.player->alive = false;
    }
    else if (entity)
    {
        entity->alive = false;
    }

    // Movement logic
    if (target_cell == Dungeon::CELL_ROCK)
    {
        if (!tunneling || hardness >= 255)
            return false;

        int new_hardness = hardness - 85;
        if (new_hardness <= 0)
        {
            g.dungeon.type_grid(target_x, target_y) = Dungeon::CELL_CORRIDOR;
            g.dungeon.hardness_grid(target_x, target_y) = 0;
            g.move_entity(x, y, target_x, target_y);
        }
        else
        {
            g.dungeon.hardness_grid(target_x, target_y) = new_hardness;
        }

        // Recalculate distance maps
        Monster::update_global_maps(g);
        return true;
    }

    // Walkable cell
    if (target_cell != Dungeon::CELL_ROCK)
    {
        g.move_entity(x, y, target_x, target_y);
        return true;
    }

    return false;
}
