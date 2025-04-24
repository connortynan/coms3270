#pragma once

#include <vector>
#include <memory>
#include <list>
#include <functional>
#include <random>

#include "entity.hpp"
#include "player.hpp"
#include "dungeon.hpp"
#include "util/event_queue.hpp"
#include "util/shadowcast.hpp"
#include "util/grid.hpp"
#include "util/filtered_view.hpp"
#include "monster_parser.hpp"
#include "object_parser.hpp"

static constexpr mapsize_t VISIBILITY_RADIUS = 3;

struct VisibilityData
{
    Dungeon::cell_type_t last_seen;
    bool visible;
};

class GameContext
{
public:
    GameContext(Dungeon::Generator::Parameters params, mapsize_t width, mapsize_t height, unsigned int num_entities, int seed = 0);

    void regenerate_dungeon();
    void set_dungeon(Dungeon &d, mapsize_t pc_x, mapsize_t pc_y);

    void add_entity(std::unique_ptr<Entity> e);
    void remove_entity(Entity *e);
    void move_entity(Entity *e,
                     mapsize_t from_x, mapsize_t from_y,
                     mapsize_t to_x, mapsize_t to_y);
    void clear_entities();

    std::vector<Entity *> entities_at(mapsize_t x, mapsize_t y) const;
    Entity *top_entity_at(mapsize_t x, mapsize_t y) const;

    void schedule_event(EventQueue::Callback cb, tick_t delay);
    void process_events();
    void flush_events();

    void update_on_change();

    VisibilityData &visibility_at(mapsize_t x, mapsize_t y);
    void quit();
    tick_t current_tick() const;

    template <typename T, typename F = std::function<bool(const T &)>>
    std::vector<T *> filter(F &&pred = [](const T &)
                            { return true; }) const;

    template <typename T>
    auto filtered(std::function<bool(const T &)> pred = [](const T &)
                  { return true; }) const
    {
        return FilteredView<T, std::vector<std::unique_ptr<Entity>>>(entities, pred);
    }

    void schedule_character_event(Character *c);
    void run_turn();

private:
    void cleanup_dead_entities();

    void update_visibility_map();
    void update_monster_tunneling_map();
    void update_monster_nontunneling_map();

    void remove_entity_from_map(Entity *e);

    void load_descriptions();

    void spawn_entity();

public:
    Player player;
    Dungeon dungeon;
    std::vector<std::unique_ptr<Entity>> entities;

public:
    bool running = true;
    Grid<std::list<Entity *>> entity_map;
    Grid<VisibilityData> visibility_map;
    Grid<unsigned int> monster_tunneling_map;
    Grid<unsigned int> monster_nontunneling_map;

private:
    EventQueue events;
    Dungeon::Generator::Parameters gen_params;

    std::vector<MonsterDesc> monster_descs;
    std::vector<ObjectDesc> object_descs;

    std::unordered_set<const ObjectDesc *> claimed_artifacts; // Permanently removed
    std::unordered_set<const MonsterDesc *> killed_uniques;   // Permanently removed
    std::unordered_set<const MonsterDesc *> spawned_uniques;  // Currently alive

    unsigned int num_entities;

    std::mt19937 rng;
};

template <typename T, typename F>
std::vector<T *> GameContext::filter(F &&pred) const
{
    std::vector<T *> result;
    for (const auto &e : entities)
    {
        if (auto *casted = e.get()->as<T>())
        {
            if (pred(*casted))
            {
                result.push_back(casted);
            }
        }
    }
    return result;
}

inline void GameContext::spawn_entity()
{
    std::uniform_int_distribution<> monster_chance(0, monster_descs.size() + object_descs.size() - 1);
    bool spawn_monster = static_cast<std::size_t>(monster_chance(rng)) < monster_descs.size();

    if (spawn_monster && !monster_descs.empty())
    {
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            const MonsterDesc &desc = monster_descs[rng() % monster_descs.size()];

            // Skip unique monsters that are killed or already spawned
            if (desc.has_ability(Monster::Abilities::UNIQUE) &&
                (killed_uniques.count(&desc) || spawned_uniques.count(&desc)))
                continue;

            if (desc.rarity < std::uniform_int_distribution<>(0, 99)(rng))
                continue;

            mapsize_t x = 0, y = 0;
            do
            {
                x = rng() % dungeon.width;
                y = rng() % dungeon.height;
            } while (dungeon.type_grid.at(x, y) == Dungeon::CELL_ROCK ||
                     top_entity_at(x, y) != nullptr);

            auto monster = desc.make_instance(x, y, rng);
            add_entity(std::move(monster));

            if (desc.has_ability(Monster::Abilities::UNIQUE))
                spawned_uniques.insert(&desc);

            return;
        }
    }
    else if (!object_descs.empty())
    {
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            const ObjectDesc &desc = object_descs[rng() % object_descs.size()];
            // Skip artifacts that are claimed
            if (desc.is_artifact && claimed_artifacts.count(&desc))
                continue;

            if (desc.rarity < std::uniform_int_distribution<>(0, 99)(rng))
                continue;

            mapsize_t x = 0, y = 0;
            do
            {
                x = rng() % dungeon.width;
                y = rng() % dungeon.height;
            } while (dungeon.type_grid.at(x, y) == Dungeon::CELL_ROCK ||
                     top_entity_at(x, y) != nullptr);

            auto object = desc.make_instance(x, y, rng);
            add_entity(std::move(object));
            return;
        }
    }
}