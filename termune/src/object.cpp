#include "object.hpp"
#include "object_parser.hpp" // for ObjectDesc

constexpr mapsize_t OBJECT_ZINDEX = 1; // Below player and monsters

Object::Object(mapsize_t x, mapsize_t y,
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
    : Entity(x, y, object_type_to_char(type), color, OBJECT_ZINDEX),
      desc(desc),
      weight(weight),
      hit(hit),
      damage(damage),
      dodge(dodge),
      defense(defense),
      speed(speed),
      attribute(attribute),
      value(value),
      type(type),
      is_artifact(is_artifact) {}

std::string_view Object::name() const
{
    return desc ? std::string_view(desc->name) : "<unnamed object>";
}

std::string_view Object::description() const
{
    return desc ? std::string_view(desc->description) : "<no description>";
}
