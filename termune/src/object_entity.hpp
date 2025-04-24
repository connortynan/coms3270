#pragma once

#include "entity.hpp"
#include "object_item.hpp"
#include "util/dice.hpp"
#include "util/colors.hpp"

struct ObjectDesc;

class ObjectEntity : public Object, public Entity
{
public:
    ObjectEntity(mapsize_t x, mapsize_t y,
                 Object::Type type,
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
                 int value);

    Object create_object() const;

    void on_collision(Entity &other) override;
};
