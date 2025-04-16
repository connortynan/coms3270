#include "entity.hpp"
#include "game_context.hpp"
#include "ui.hpp"

void Entity::render(UiManager &ui, unsigned char color_index) const
{
    if (!active || colors.empty())
        return;

    WINDOW *w = ui.win(UiManager::WindowID::DUNGEON);

    mvwaddch(w, y, x, symbol | colors[color_index >= colors.size() ? 0 : color_index]);
}

bool Entity::move(int dx, int dy, GameContext &g, bool force)
{
    mapsize_t target_x = x + dx;
    mapsize_t target_y = y + dy;

    if (!g.dungeon.in_bounds(target_x, target_y))
        return false;

    if (!force && g.dungeon.type_grid.at(target_x, target_y) == Dungeon::CELL_ROCK)
        return false;

    for (auto &entity : g.entities_at(target_x, target_y))
    {
        if (entity == this)
            continue;

        entity->onCollision(*this);
    }

    g.move_entity(this, x, y, target_x, target_y);

    return true;
}
