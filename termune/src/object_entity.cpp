#include "object_entity.hpp"

#include <vector>

#include "object_item.hpp"
#include "player.hpp"
#include "entity.hpp"
#include "ui.hpp"

constexpr mapsize_t OBJECT_ZINDEX = 1; // Below player and monsters

ObjectEntity::ObjectEntity(mapsize_t x, mapsize_t y,
                           Type type,
                           std::vector<short> color,
                           bool is_artifact,
                           const ObjectDesc *desc,
                           int weight,
                           int hit,
                           const Dice &damage,
                           int dodge,
                           int defense,
                           int speed,
                           int attribute,
                           int value)
    : Object(type, is_artifact, desc, weight, hit, damage, dodge, defense, speed, attribute, value),
      Entity(x, y, object_type_to_char(type), color, OBJECT_ZINDEX) {}

Object ObjectEntity::create_object() const
{
  return Object(type, is_artifact, desc, weight, hit, damage, dodge, defense, speed, attribute, value);
}

void ObjectEntity::on_collision(Entity &other)
{
  Player *player = other.as<Player>();
  if (player)
  {
    int idx = player->pickup(this);
    active = false;

    if (idx != -1)
    {
      player->ui->display_message("Picked up %s", std::string(name()).c_str());
    }
    else
    {
      player->ui->display_message("Inventory full!");
    }
  }
}
