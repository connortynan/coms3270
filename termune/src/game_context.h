#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "entity.h"
#include "dungeon.h"
#include "util/event_queue.h"
#include "util/shadowcast.h"

static constexpr mapsize_t VISIBILITY_RADIUS = 3;

struct VisibilityData
{
    Dungeon::cell_type_t last_seen;
    bool visible;
};

class GameContext
{
public:
    GameContext(Dungeon::Generator::Parameters params, entity_id_t num_monsters, mapsize_t dungeon_width, mapsize_t dungeon_height)
        : dungeon(dungeon_width, dungeon_height),
          entity_grid(dungeon_width, dungeon_height),
          visibility_grid(dungeon_width, dungeon_height, {Dungeon::CELL_ROCK, false}),
          monster_tunneling_map(dungeon_width, dungeon_height, 0),
          monster_nontunneling_map(dungeon_width, dungeon_height, 0),
          monster_line_of_sight_map(dungeon_width, dungeon_height, false),
          gen_params(params),
          next_entity_id(1),
          num_monsters(num_monsters)
    {
        auto p = std::make_shared<Player>(0, 0);
        p->id = ENTITY_PLAYER;
        player = p;
        entity_grid.at(0, 0) = p;
    }

    entity_id_t add_monster(std::shared_ptr<Monster> m)
    {
        assert(entity_grid.at(m->x, m->y) == nullptr && "Grid cell already occupied");
        m->id = next_entity_id++;

        alive_monsters.insert(m);

        entity_grid.at(m->x, m->y) = m;
        return m->id;
    }

    void remove_entity_in_grid(mapsize_t x, mapsize_t y)
    {
        auto &slot = entity_grid.at(x, y);
        if (!slot)
            return;

        slot.reset(); // shared_ptr will delete if last ref
    }

    void move_entity(mapsize_t from_x, mapsize_t from_y, mapsize_t to_x, mapsize_t to_y)
    {
        if (from_x == to_x && from_y == to_y)
            return;

        assert(dungeon.in_bounds(to_x, to_y));
        auto &from = entity_grid.at(from_x, from_y);
        assert(from && "No entity at source position");

        auto &killed = entity_grid.at(to_x, to_y);
        if (killed)
        {
            killed->alive = false;
            remove_entity_in_grid(to_x, to_y);
        }

        from->x = to_x;
        from->y = to_y;
        entity_grid.at(to_x, to_y) = std::move(from);
    }

    void cleanup_dead_entities()
    {
        for (auto it = alive_monsters.begin(); it != alive_monsters.end();)
        {
            auto &m = *it;
            if (!m->alive)
            {
                if (entity_grid.at(m->x, m->y) == m)
                    remove_entity_in_grid(m->x, m->y);
                it = alive_monsters.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    std::shared_ptr<Entity> entity_at(mapsize_t x, mapsize_t y)
    {
        if (!dungeon.in_bounds(x, y))
            return nullptr;
        return entity_grid.at(x, y);
    }

    std::shared_ptr<const Entity> entity_at(mapsize_t x, mapsize_t y) const
    {
        if (!dungeon.in_bounds(x, y))
            return nullptr;
        return entity_grid.at(x, y);
    }

    void regenerate_dungeon()
    {
        Dungeon::Generator::generate_dungeon(dungeon, gen_params, 0);
        set_dungeon(dungeon, dungeon.rooms[0].center_x, dungeon.rooms[0].center_y);
        visibility_grid.fill({Dungeon::CELL_ROCK, false});
        update_on_change();
    }

    void set_dungeon(Dungeon &d, mapsize_t pc_x, mapsize_t pc_y)
    {
        dungeon = d;
        entity_grid.fill(nullptr);
        alive_monsters.clear();

        // player entity
        auto p = std::make_shared<Player>(pc_x, pc_y);
        p->id = ENTITY_PLAYER;
        player = p;
        entity_grid.at(pc_x, pc_y) = p;

        for (int i = 0; i < num_monsters; i++)
        {
            mapsize_t monster_x, monster_y;
            do
            {
                monster_x = rand() % dungeon.width;
                monster_y = rand() % dungeon.height;
            } while (dungeon.type_grid.at(monster_x, monster_y) == Dungeon::CELL_ROCK || entity_grid.at(monster_x, monster_y) != nullptr);

            uint8_t flags = rand() % 0xF;
            auto m = std::make_shared<Monster>(
                monster_x, monster_y,
                (rand() % 15) + 5,
                0, // dummy id (will get overwritten in add_monster)
                flags);
            add_monster(m);
        }
    }

    void add_entity_event(entity_id_t id, tick_t delay)
    {
        events.add(id, delay);
    }

    void process_events()
    {
        std::vector<entity_id_t> entities = events.resolve_events_until(ENTITY_PLAYER);
        for (const entity_id_t id : entities)
        {
            if (id == ENTITY_PLAYER && player->alive)
            {
                add_entity_event(id, 1000 / player->speed);
                break;
            }
            for (auto &m : alive_monsters)
            {
                if (m->alive && id == m->id)
                {
                    int dx, dy;
                    m->get_desired_move(dx, dy, *this);
                    m->move(dx, dy, *this);
                    add_entity_event(id, 1000 / m->speed);
                    break;
                }
            }
        }
        cleanup_dead_entities();
        running = alive_monsters.size() > 0 && player->alive;
    }

    void update_visibility()
    {
        ShadowCast::Lightmap solid(dungeon.width, dungeon.height);

        for (mapsize_t y = 0; y < dungeon.height; y++)
            for (mapsize_t x = 0; x < dungeon.width; x++)
            {
                solid.at(x, y) = dungeon.type_grid.at(x, y) == Dungeon::CELL_ROCK;
            }

        ShadowCast::Lightmap visible_map = ShadowCast::solve_lightmap(solid, player->x, player->y, VISIBILITY_RADIUS);

        for (mapsize_t y = 0; y < visible_map.height(); y++)
            for (mapsize_t x = 0; x < visible_map.width(); x++)
            {
                visibility_grid.at(x, y).visible = visible_map.at(x, y);

                if (visible_map.at(x, y))
                    visibility_grid.at(x, y).last_seen = dungeon.type_grid.at(x, y);
            }
    }

    void update_on_change()
    {
        update_visibility();
        Monster::update_global_maps(*this);
    }

    VisibilityData &visibility_at(const mapsize_t x, const mapsize_t y) { return visibility_grid.at(x, y); }

    void quit() { running = false; }
    tick_t current_tick() const { return events.current_tick(); };

public:
    Dungeon dungeon;
    std::shared_ptr<Player> player;
    std::unordered_set<std::shared_ptr<Monster>> alive_monsters;
    bool running = true;

private:
    Grid<std::shared_ptr<Entity>> entity_grid;
    Grid<VisibilityData> visibility_grid;
    Grid<unsigned int> monster_tunneling_map;
    Grid<unsigned int> monster_nontunneling_map;
    Grid<unsigned char> monster_line_of_sight_map; // right now equivalent to visibilty grid
    EventQueue<entity_id_t> events;
    Dungeon::Generator::Parameters gen_params;
    entity_id_t next_entity_id;
    entity_id_t num_monsters;

    friend class Monster;
};
