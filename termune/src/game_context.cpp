#include "game_context.hpp"

#include <stdexcept>
#include <algorithm>

#include "util/pathing.hpp"
#include "monster_parser.hpp"
#include "object_parser.hpp"
#include "util/fs.hpp"

GameContext::GameContext(Dungeon::Generator::Parameters params, mapsize_t width, mapsize_t height, unsigned int num_entities, int seed)
    : player(0, 0),
      dungeon(width, height),
      entity_map(width, height),
      visibility_map(width, height, {Dungeon::CELL_ROCK, false}),
      monster_tunneling_map(width, height, 0),
      monster_nontunneling_map(width, height, 0),
      gen_params(params),
      num_entities(num_entities),
      rng(seed == 0 ? std::random_device{}() : seed)
{
    load_descriptions();
}

void GameContext::regenerate_dungeon()
{
    Dungeon::Generator::generate_dungeon(dungeon, gen_params, 0);
    set_dungeon(dungeon, dungeon.rooms[0].center_x, dungeon.rooms[0].center_y);
}

void GameContext::set_dungeon(Dungeon &d, mapsize_t pc_x, mapsize_t pc_y)
{
    dungeon = d;
    entities.clear();
    entity_map.fill({});
    player.x = pc_x;
    player.y = pc_y;

    // Reset unique tracking for this floor
    spawned_uniques.clear();
    for (unsigned int i = 0; i < num_entities; ++i)
    {
        spawn_entity();
    }

    events.flush();
    schedule_character_event(&player);
    for (auto *c : filter<Character>())
    {
        schedule_character_event(c);
    }

    visibility_map.fill({Dungeon::CELL_ROCK, false});
    update_on_change();
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

void GameContext::remove_entity_from_map(Entity *e)
{
    auto &list = entity_map.at(e->x, e->y);
    list.remove(e);
}

void GameContext::remove_entity(Entity *e)
{
    auto it = std::find_if(entities.begin(), entities.end(),
                           [&](const std::unique_ptr<Entity> &entity)
                           { return entity.get() == e; });
    if (it != entities.end())
        entities.erase(it);
    else
        return; // Entity not found

    remove_entity_from_map(e);
}

void GameContext::clear_entities()
{
    entities.clear();
    entity_map.fill({});
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

void GameContext::cleanup_dead_entities()
{
    for (auto it = entities.begin(); it != entities.end();)
    {
        if (!(*it)->active)
        {
            remove_entity_from_map(it->get());
            it = entities.erase(it);
        }
        else
        {
            ++it;
        }
    }
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

Entity *GameContext::top_entity_at(mapsize_t x, mapsize_t y) const
{
    auto &list = entity_map.at(x, y);
    if (list.empty())
        return nullptr;
    return list.back();
}

void GameContext::schedule_event(EventQueue::Callback cb, tick_t delay)
{
    events.add(std::move(cb), delay);
}

void GameContext::schedule_character_event(Character *c)
{
    if (!c || !c->active)
        return;

    schedule_event(
        [this, c]()
        {
            bool is_player = c->as<Player>() != nullptr;

            if (c->health <= 0)
            {
                c->active = false;
                return is_player; // stop processing events
            }

            schedule_character_event(c);

            if (!c->active)
                return false;

            int dx = 0, dy = 0;
            bool force = false;

            if (is_player)
            {
                dx = dy = 0;
                force = false;
                c->move(dx, dy, *this, force);
                if (!running)
                    return true;
                update_on_change();
                return true; // stop processing events
            }
            else
            {
                c->as<Monster>()->get_desired_move(dx, dy, force, *this);
                c->move(dx, dy, *this, force);
                return false; // continue processing events
            }
        },
        c->event_delay());
}

void GameContext::run_turn()
{
    cleanup_dead_entities();
    events.process();
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
    update_visibility_map();
    update_monster_tunneling_map();
    update_monster_nontunneling_map();
}

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

void GameContext::load_descriptions()
{
    fs::ensure_data_dir_exists();

    std::string monster_path = fs::join(fs::rlg327_data_dir(), "monster_desc.txt");
    std::string object_path = fs::join(fs::rlg327_data_dir(), "object_desc.txt");

    monster_descs = std::move(MonsterParser::load_from_file(monster_path));
    object_descs = std::move(ObjectParser::load_from_file(object_path));
}

namespace // hide from other translation units
{
    class DistanceNode : public Pathing::Node
    {
    public:
        mapsize_t x, y;
        Grid<Dungeon::cell_hardness_t> *weights;
        mapsize_t w, h;

        DistanceNode(mapsize_t x, mapsize_t y, Grid<Dungeon::cell_hardness_t> *map)
            : x(x), y(y), weights(map), w(map->width()), h(map->height()) {}

        std::vector<std::size_t> get_neighbors() const override
        {
            std::vector<std::size_t> neighbors;
            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dy == 0)
                        continue;
                    mapsize_t nx = x + dx, ny = y + dy;
                    if (nx < w && ny < h && weights->at(nx, ny) < 255)
                        neighbors.push_back(ny * w + nx);
                }
            }
            return neighbors;
        }

        unsigned long movement_cost_to(const Node &n) const override
        {
            const auto &other = static_cast<const DistanceNode &>(n);
            return 1 + weights->at(other.x, other.y) / 85;
        }

        bool is_goal() const override { return false; }
    };
} // namespace

void GameContext::update_visibility_map()
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
}

void GameContext::update_monster_tunneling_map()
{
    auto &weights = dungeon.hardness_grid;

    std::vector<std::unique_ptr<Pathing::Node>> nodes;
    for (mapsize_t y = 0; y < dungeon.height; ++y)
        for (mapsize_t x = 0; x < dungeon.width; ++x)
            nodes.push_back(std::make_unique<DistanceNode>(x, y, &weights));

    std::size_t start = player.x + player.y * dungeon.width;
    Pathing::solve(nodes, start);

    for (std::size_t i = 0; i < nodes.size(); ++i)
        monster_tunneling_map(i % dungeon.width, i / dungeon.width) = static_cast<uint32_t>(nodes[i]->cost);
}

void GameContext::update_monster_nontunneling_map()
{
    Grid<Dungeon::cell_hardness_t> weights(dungeon.width, dungeon.height);

    for (mapsize_t y = 0; y < dungeon.height; ++y)
        for (mapsize_t x = 0; x < dungeon.width; ++x)
            weights.at(x, y) = dungeon.type_grid(x, y) == Dungeon::CELL_ROCK ? 255 : 0;

    std::vector<std::unique_ptr<Pathing::Node>> nodes;
    for (mapsize_t y = 0; y < dungeon.height; ++y)
        for (mapsize_t x = 0; x < dungeon.width; ++x)
            nodes.push_back(std::make_unique<DistanceNode>(x, y, &weights));

    std::size_t start = player.x + player.y * dungeon.width;
    Pathing::solve(nodes, start);

    for (std::size_t i = 0; i < nodes.size(); ++i)
        monster_nontunneling_map(i % dungeon.width, i / dungeon.width) = static_cast<uint32_t>(nodes[i]->cost);
}
