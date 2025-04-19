#include "monster.hpp"
#include "game_context.hpp"
#include "util/pathing.hpp"
#include "util/shadowcast.hpp"
#include "dungeon.hpp"

void Monster::update_sight(mapsize_t x, mapsize_t y, GameContext &g)
{
    has_line_of_sight = g.visibility_at(x, y).visible;
    if (has_line_of_sight)
    {
        target_x = g.player.x;
        target_y = g.player.y;
    }
}

void Monster::get_desired_move(int &dx, int &dy, bool &force, GameContext &g) const
{
    force = false;
    dx = dy = 0;

    if (has(Abilities::ERRATIC) && rand() % 2)
    {
        dx = (rand() % 3) - 1;
        dy = (rand() % 3) - 1;
        return;
    }

    if (has_line_of_sight && target_x != EMPTY_TARGET && target_y != EMPTY_TARGET)
    {
        dx = (target_x == x ? 0 : (target_x > x ? 1 : -1));
        dy = (target_y == y ? 0 : (target_y > y ? 1 : -1));
        return;
    }

    if (has(Abilities::INTELLIGENT) && has(Abilities::TELEPATHIC))
    {
        const auto &dist_map = has(Abilities::TUNNELING) ? g.monster_tunneling_map : g.monster_nontunneling_map;

        mapsize_t best_x = x, best_y = y;
        uint32_t best_cost = dist_map(x, y);
        int start = rand() % 8;
        const int offsets[8][2] = {
            {-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

        for (int i = 0; i < 8; ++i)
        {
            auto nx = static_cast<std::size_t>(x + offsets[(start + i) % 8][0]);
            auto ny = static_cast<std::size_t>(y + offsets[(start + i) % 8][1]);
            if (nx >= dist_map.width() || ny >= dist_map.height())
                continue;
            uint32_t cost = dist_map(nx, ny);
            if (cost < best_cost)
            {
                best_cost = cost;
                best_x = nx;
                best_y = ny;
            }
        }

        dx = static_cast<int>(best_x) - static_cast<int>(x);
        dy = static_cast<int>(best_y) - static_cast<int>(y);
        return;
    }

    if (has(Abilities::INTELLIGENT) && !has(Abilities::TELEPATHIC))
    {
        if (target_x != EMPTY_TARGET && target_y != EMPTY_TARGET)
        {
            dx = (target_x == x ? 0 : (target_x > x ? 1 : -1));
            dy = (target_y == y ? 0 : (target_y > y ? 1 : -1));
        }
        return;
    }

    if (!has(Abilities::INTELLIGENT) && has(Abilities::TELEPATHIC))
    {
        dx = (target_x == x ? 0 : (target_x > x ? 1 : -1));
        dy = (target_y == y ? 0 : (target_y > y ? 1 : -1));
        return;
    }

    dx = dy = 0;
}
bool Monster::move(int dx, int dy, GameContext &g, bool force)
{
    mapsize_t nx = x + dx;
    mapsize_t ny = y + dy;

    if (!g.dungeon.in_bounds(nx, ny))
        return false;

    auto cell = g.dungeon.type_grid(nx, ny);
    auto hardness = g.dungeon.hardness_grid(nx, ny);

    if (cell == Dungeon::CELL_ROCK)
    {
        if (!has(Abilities::TUNNELING) || hardness >= 255)
            return false;

        int new_hardness = hardness - 85;
        if (new_hardness <= 0)
        {
            g.dungeon.type_grid(nx, ny) = Dungeon::CELL_CORRIDOR;
            g.dungeon.hardness_grid(nx, ny) = 0;
            g.update_on_change(); // update visibility and distance maps
        }
        else
        {
            g.dungeon.hardness_grid(nx, ny) = new_hardness;
            g.update_on_change(); // update visibility and distance maps
            return true;          // did some mining but not enough to pass through
        }
    }

    return Entity::move(dx, dy, g, force);
}

std::string_view Monster::name() const
{
    return desc ? std::string_view(desc->name.data(), desc->name.size()) : "Unknown Monster";
}
std::string_view Monster::description() const
{
    return desc ? std::string_view(desc->description.data(), desc->description.size()) : "No description available";
}