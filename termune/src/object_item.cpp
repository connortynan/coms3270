#include "object_item.hpp"

#include "object_parser.hpp"
#include "object_entity.hpp"

Object::Object(Object::Type type,
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
    : desc(desc), weight(weight), hit(hit), damage(damage),
      dodge(dodge), defense(defense), speed(speed),
      attribute(attribute), value(value),
      type(type), is_artifact(is_artifact) {}

ObjectEntity Object::to_entity(mapsize_t x, mapsize_t y) const
{
    return ObjectEntity(x, y, type, desc->colors, is_artifact, desc,
                        weight, hit, damage,
                        dodge, defense, speed,
                        attribute, value);
}

std::string_view Object::name() const
{
    return (desc ? desc->name : "<unnamed item>");
}
std::string_view Object::description() const
{
    return (desc ? desc->description : "<no description>");
}

bool Object::is(Type t) const
{
    if (t == Object::TYPE_NONE)
        return type == Object::TYPE_NONE;
    return (type & t) != 0;
}