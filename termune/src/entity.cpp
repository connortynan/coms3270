#include "entity.hpp"
#include "game_context.hpp"
#include "ui.hpp"

void Entity::render(ui::Context &ui, std::size_t color_index) const
{
    if (!active || colors.empty())
        return;

    WINDOW *w = ui.win(ui::Context::WindowID::DUNGEON);
    short color = colors[color_index % colors.size()];
    wattron(w, COLOR_PAIR(color));
    mvwaddch(w, y, x, symbol);
    wattroff(w, COLOR_PAIR(color));
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

        entity->on_collision(*this);
    }

    g.move_entity(this, x, y, target_x, target_y);

    return true;
}
