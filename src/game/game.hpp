#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <set>
#include <memory>
#include <game/actions.hpp>
#include <game/view.hpp>
#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  class ItemType {
    public:
    std::string name;

    nc::Entity ext;

    ItemType(unsigned int id) : _id(id) {}

    unsigned int id() const { return _id; }

    private:
    unsigned int _id;
  };
  class Item {
    public:
    unsigned int quantity = 0;

    Item() = default;
    Item(const ItemType * type) : _type(type) {}

    const ItemType * type() const { return _type; }

    private:
    const ItemType * _type = nullptr;
  };

  struct WorldItem {
    Vec2i position;
    Item item;
  };

  struct Space {
    Map<unsigned int> tiles;
    Map<unsigned int> opaque;

    std::vector<WorldItem> items;
    std::set<nc::Id> entities;

    bool is_passable(Vec2i pos);
  };

  class Entity {
    public:
    std::string name;

    Vec2i position;
    nc::Id space_id;

    std::vector<Item> inventory;
    std::map<std::string, Item> equipment;

    nc::Entity ext;
  };

  struct World {
    nc::IdSet<Entity> entities;
    nc::IdSet<Space> spaces;
  };


  class SpawnHandler {
    public:
    virtual void on_spawn(nc::Id eid) = 0;
  };
  class DespawnHandler {
    public:
    virtual void on_despawn(nc::Id eid) = 0;
  };
  class MoveHandler {
    public:
    virtual void on_move(nc::Id eid, Vec2i from, Vec2i to) = 0;
  };
  class FOVUpdateHandler {
    public:
    virtual void on_fov_update(nc::Id eid) = 0;
  };

  struct EngineView {
  };

  // Systems exist to split up the code in World
  class ViewSys : public SpawnHandler,
                  public DespawnHandler,
                  public MoveHandler,
                  public FOVUpdateHandler {
    public:
    ViewSys(View & view, const World & world) : view(view), world(world) {}

    void on_spawn_space(nc::Id sid);
    void on_despawn_space(nc::Id sid);
    void on_tile_update(nc::Id sid, Vec2i pos, unsigned int new_val);

    void on_spawn(nc::Id eid) override;
    void on_despawn(nc::Id eid) override;
    void on_move(nc::Id eid, Vec2i from, Vec2i to) override;

    void on_fov_update(nc::Id eid) override;

    private:
    View & view;
    const World & world;
  };

  class TurnSys {
    public:
    void add_turn(nc::Id eid, unsigned int speed);
    void remove_turn(nc::Id eid);
    void tick();

    std::pair<bool, nc::Id> whos_turn() const;
    void finish_turn();

    private:
    struct Turn {
      nc::Id eid;
      unsigned int energy;
      unsigned int speed;
    };

    std::map<decltype(nc::Id::idx), Turn> turns;
  };

  class Action;

  class Engine {
    public:
    Engine(View & view)
      : view_sys(view, world)
    {}

    std::pair<bool, nc::Id> step(unsigned int max_ticks);
    void complete_turn(const Action & action);

    nc::Id create_hero();
    nc::Id create_badguy(nc::Id kill_eid);

    nc::Id create_space(Vec2u size);
    void        destroy_space(nc::Id sid);

    void spawn(nc::Id eid, nc::Id sid, Vec2i pos);
    void despawn(nc::Id eid);

    void move_attack(nc::Id eid, Vec2i delta);
    void wait(nc::Id eid);

    private:
    World world;

    ViewSys view_sys;
    TurnSys turn_sys;

    void tick();
  };
}

#endif
