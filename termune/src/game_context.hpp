#pragma once

#include <vector>
#include <memory>
#include <list>
#include <functional>

#include "entity.hpp"
#include "player.hpp"
#include "dungeon.hpp"
#include "util/event_queue.hpp"
#include "util/shadowcast.hpp"
#include "util/grid.hpp"

static constexpr mapsize_t VISIBILITY_RADIUS = 3;

struct VisibilityData
{
    Dungeon::cell_type_t last_seen;
    bool visible;
};

class GameContext
{
public:
    GameContext(Dungeon::Generator::Parameters params, mapsize_t width, mapsize_t height);

    void regenerate_dungeon();
    void set_dungeon(Dungeon &d, mapsize_t pc_x, mapsize_t pc_y);

    void add_entity(std::unique_ptr<Entity> e);
    void remove_entity(Entity *e);
    void move_entity(Entity *e,
                     mapsize_t from_x, mapsize_t from_y,
                     mapsize_t to_x, mapsize_t to_y);

    std::vector<Entity *> entities_at(mapsize_t x, mapsize_t y) const;

    void schedule_event(EventQueue::Callback cb, tick_t delay);
    void process_events();
    void flush_events();

    void update_on_change();

    VisibilityData &visibility_at(mapsize_t x, mapsize_t y);
    void quit();
    tick_t current_tick() const;

    template <typename T, typename F = std::function<bool(const T &)>>
    std::vector<T *> filter(F &&pred = [](const T &)
                            { return true; });

private:
    void insert_entity_sorted(Entity *e, mapsize_t x, mapsize_t y);

    void cleanup_dead_entities();

    void update_visibility_map();
    void update_monster_tunneling_map();
    void update_monster_nontunneling_map();
    void update_monster_line_of_sight_map();

public:
    Player player;
    Dungeon dungeon;
    std::vector<std::unique_ptr<Entity>> entities;

private:
    bool running = true;
    Grid<std::list<Entity *>> entity_map;
    Grid<VisibilityData> visibility_map;
    Grid<unsigned int> monster_tunneling_map;
    Grid<unsigned int> monster_nontunneling_map;
    Grid<unsigned char> monster_line_of_sight_map;
    EventQueue events;
    Dungeon::Generator::Parameters gen_params;
};

template <typename T, typename F>
std::vector<T *> GameContext::filter(F &&pred)
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
