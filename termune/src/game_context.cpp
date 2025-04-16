#include "game_context.hpp"
#include <stdexcept>
#include <algorithm>

GameContext::GameContext(Dungeon::Generator::Parameters params, mapsize_t width, mapsize_t height)
    : dungeon(width, height),
      entity_map(width, height),
      visibility_map(width, height, {Dungeon::CELL_ROCK, false}),
      monster_tunneling_map(width, height, 0),
      monster_nontunneling_map(width, height, 0),
      monster_line_of_sight_map(width, height, false),
      gen_params(params),
      player(0, 0) {}

void GameContext::regenerate_dungeon()
{
    Dungeon::Generator::generate_dungeon(dungeon, gen_params, 0);
    set_dungeon(dungeon, dungeon.rooms[0].center_x, dungeon.rooms[0].center_y);
    visibility_map.fill({Dungeon::CELL_ROCK, false});
    update_on_change();
}

void GameContext::set_dungeon(Dungeon &d, mapsize_t pc_x, mapsize_t pc_y)
{
    dungeon = d;
    entities.clear();
    entity_map.fill({});
    player.x = pc_x;
    player.y = pc_y;
}

void GameContext::add_entity(std::unique_ptr<Entity> e)
{
    Entity *raw = e.get();
    entities.push_back(std::move(e));

    auto &list = entity_map.at(raw->x, raw->y);
    auto it = std::find_if(list.begin(), list.end(), [&](Entity *other)
                           { return raw->z < other->z; });
    list.insert(it, raw);
}

void GameContext::move_entity(Entity *e,
                              mapsize_t from_x, mapsize_t from_y,
                              mapsize_t to_x, mapsize_t to_y)
{
    entity_map.at(from_x, from_y).remove(e);

    auto &list = entity_map.at(to_x, to_y);
    auto it = std::find_if(list.begin(), list.end(), [&](Entity *other)
                           { return e->z < other->z; });
    list.insert(it, e);

    e->x = to_x;
    e->y = to_y;
}

std::vector<Entity *> GameContext::entities_at(mapsize_t x, mapsize_t y) const
{
    std::vector<Entity *> result;
    for (Entity *e : entity_map.at(x, y))
    {
        result.push_back(e);
    }
    return result;
}

void GameContext::schedule_event(EventQueue::Callback cb, tick_t delay)
{
    events.add(std::move(cb), delay);
}

void GameContext::process_events()
{
    events.process();
}

void GameContext::flush_events()
{
    events.flush();
}

void GameContext::update_on_change()
{
    ShadowCast::Lightmap solid(dungeon.width, dungeon.height);
    for (mapsize_t y = 0; y < dungeon.height; ++y)
        for (mapsize_t x = 0; x < dungeon.width; ++x)
            solid.at(x, y) = dungeon.type_grid.at(x, y) == Dungeon::CELL_ROCK;

    auto lightmap = ShadowCast::solve_lightmap(solid, player.x, player.y, VISIBILITY_RADIUS);
    for (mapsize_t y = 0; y < dungeon.height; ++y)
        for (mapsize_t x = 0; x < dungeon.width; ++x)
        {
            visibility_map.at(x, y).visible = lightmap.at(x, y);
            if (lightmap.at(x, y))
                visibility_map.at(x, y).last_seen = dungeon.type_grid.at(x, y);
        }

    update_monster_tunneling_map();
    update_monster_nontunneling_map();
    update_monster_line_of_sight_map();
}

void GameContext::update_monster_tunneling_map() {}
void GameContext::update_monster_nontunneling_map() {}
void GameContext::update_monster_line_of_sight_map() {}

VisibilityData &GameContext::visibility_at(mapsize_t x, mapsize_t y)
{
    return visibility_map.at(x, y);
}

void GameContext::quit()
{
    running = false;
}

tick_t GameContext::current_tick() const
{
    return events.current_tick();
}
