#pragma once

#include <vector>
#include <string>
#include "types.hpp"
#include "util/colors.hpp"

class GameContext;

namespace ui
{
    class Context;
}

class Entity
{
public:
    mapsize_t x, y;
    char symbol;
    int z;

    std::vector<short> colors;
    bool active = true;

    Entity(mapsize_t x, mapsize_t y, char sym, std::vector<short> cols, int zindex = 0)
        : x(x), y(y), symbol(sym), z(zindex), colors(std::move(cols)) {}

    virtual ~Entity() = default;

    // Called each game tick (characters override this)
    virtual void update(GameContext &g) {}

    // Move this entity on the map
    virtual bool move(int dx, int dy, GameContext &g, bool force = false);

    // Render the entity on the screen, with the specified color index
    // (0 = first color in the vector, 1 = second, etc.)
    // If the color index is out of bounds, it will modulo based on colors.size()
    virtual void render(ui::Context &ui, std::size_t color_index) const;

    // Called when another entity moves into this one
    virtual void on_collision(Entity &other) {}

    // Downcasting helper functions (used to downcast to derived types)
    // These are not safe, so wrap in a check for nullptr
    // `if (auto &derived = entity->as<DerivedType>()) { ... }` is safe
    template <typename T>
    T *as()
    {
        return dynamic_cast<T *>(this);
    }

    template <typename T>
    const T *as() const
    {
        return dynamic_cast<const T *>(this);
    }
};
