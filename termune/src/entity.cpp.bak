#include "entity.hpp"

#include "ui.hpp"

bool Entity::move(int dx, int dy, GameContext &g, bool force)
{
    mapsize_t target_x = x + dx;
    mapsize_t target_y = y + dy;
    if (!g.dungeon.in_bounds(target_x, target_y))
        return false;
    if (!force && g.dungeon.type_grid.at(target_x, target_y) == Dungeon::CELL_ROCK)
        return false;
    g.move_entity(x, y, target_x, target_y);
    return true;
}