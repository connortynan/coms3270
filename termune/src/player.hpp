#pragma once

#include <array>

#include "character.hpp"
#include "util/colors.hpp"

#include "object_item.hpp"
#include "object_entity.hpp"

class GameContext;

class Player : public Character
{
public:
    Player(mapsize_t x, mapsize_t y, ui::Context *ui = nullptr);

    bool move(int, int, GameContext &g, bool) override;

    std::string_view name() const override;
    std::string_view description() const override;

    // Return index of item in inventory, or -1 if not picked up
    int pickup(ObjectEntity *o);
    int pickup(Object *o);

    void on_collision(Entity &other) override;

    // Swaps items in inventory and equipment, return true if indeces are valid
    bool equip(int inventory_index, int equipment_index);

    ui::Context *ui = nullptr;

public:
    std::array<Object, 10> inventory;

    // 0: Weapon
    // 1: Offhand
    // 2: Ranged
    // 3: Armor
    // 4: Helmet
    // 5: Cloak
    // 6: Gloves
    // 7: Boots
    // 8: Amulet
    // 9: Light
    // 10: Ring 1
    // 11: Ring 2
    std::array<Object, 12> equipment;
};