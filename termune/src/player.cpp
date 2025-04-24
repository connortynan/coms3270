#include "player.hpp"

#include "game_context.hpp"
#include "object_item.hpp"
#include "monster.hpp"
#include "ui.hpp"

constexpr mapsize_t PLAYER_ZINDEX = 3; // Above monsters and items

Player::Player(mapsize_t x, mapsize_t y, ui::Context *ui)
    : Character(x, y, '@', {COLOR_WHITE}, 10, 100, Dice{1, 1, 1}, PLAYER_ZINDEX), ui(ui) {}

bool Player::move(int, int, GameContext &g, bool)
{
    int dx, dy;
    bool force;

    while (g.running && ui->running)
    {
        ui->wait_for_input(dx, dy, force);

        // Try the move, if invalid, show message and stay in the loop
        if (Character::move(dx, dy, g, force))
        {
            ui->display_message(" "); // clear message
            return true;
        }
        // If not successful, show message and repeat
        ui->display_message("You can't move there!");
    }
    return true;
}

void Player::on_collision(Entity &other)
{
    if (auto *object = other.as<Monster>())
    {
        object->health -= damage.roll();
        for (size_t i = 0; i < inventory.size(); ++i)
        {
            if (!inventory[i].is(Object::TYPE_NONE))
            {
                object->health -= inventory[i].damage.roll();
                break;
            }
        }
    }
}

int Player::pickup(ObjectEntity *o)
{
    for (int i = 0; i < static_cast<int>(inventory.size()); ++i)
    {
        if (inventory[i].is(Object::TYPE_NONE))
        {
            inventory[i] = o->create_object();
            return i;
        }
    }
    return -1; // Inventory full
}

int Player::pickup(Object *o)
{
    for (int i = 0; i < static_cast<int>(inventory.size()); ++i)
    {
        if (inventory[i].is(Object::TYPE_NONE))
        {
            inventory[i] = *o;
            return i;
        }
    }
    return -1; // Inventory full
}

bool Player::equip(int inventory_index, int equipment_index)
{
    if (inventory_index < 0 || inventory_index >= static_cast<int>(inventory.size()) ||
        equipment_index < 0 || equipment_index >= static_cast<int>(equipment.size()))
    {
        return false;
    }

    Object inv_item = inventory[inventory_index];
    if (inv_item.is(Object::TYPE_NONE))
        return false;

    // Define expected type per equipment slot index
    constexpr std::array<Object::Type, 12> expected_types = {
        Object::TYPE_WEAPON,
        Object::TYPE_OFFHAND,
        Object::TYPE_RANGED,
        Object::TYPE_ARMOR,
        Object::TYPE_HELMET,
        Object::TYPE_CLOAK,
        Object::TYPE_GLOVES,
        Object::TYPE_BOOTS,
        Object::TYPE_AMULET,
        Object::TYPE_LIGHT,
        Object::TYPE_RING,
        Object::TYPE_RING};

    Object::Type expected = expected_types[equipment_index];

    if (!inv_item.is(expected))
        return false;

    // Swap
    std::swap(inventory[inventory_index], equipment[equipment_index]);
    return true;
}

std::string_view Player::name() const
{
    return "Player";
}

std::string_view Player::description() const
{
    return "You are the player character.";
}