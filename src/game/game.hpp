#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <set>
#include <memory>
#include <game/view.hpp>
#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  class ItemType {
    public:
    newcore::Entity ext;

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
    std::set<newcore::Id> entities;

    bool is_passable(Vec2i pos);
  };

  class Entity {
    public:
    Vec2i position;
    newcore::Id space_id;

    std::vector<Item> inventory;
    std::map<std::string, Item> equipment;

    newcore::Entity ext;
  };

  struct World {
    newcore::IdSet<Entity> entities;
    newcore::IdSet<Space> spaces;
  };


  class SpawnHandler {
    public:
    virtual void on_spawn(newcore::Id eid) = 0;
  };
  class DespawnHandler {
    public:
    virtual void on_despawn(newcore::Id eid) = 0;
  };
  class MoveHandler {
    public:
    virtual void on_move(newcore::Id eid, Vec2i from, Vec2i to) = 0;
  };
  class FOVUpdateHandler {
    public:
    virtual void on_fov_update(newcore::Id eid) = 0;
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

    void on_spawn_space(newcore::Id sid);
    void on_despawn_space(newcore::Id sid);
    void on_tile_update(newcore::Id sid, Vec2i pos, unsigned int new_val);

    void on_spawn(newcore::Id eid) override;
    void on_despawn(newcore::Id eid) override;
    void on_move(newcore::Id eid, Vec2i from, Vec2i to) override;

    void on_fov_update(newcore::Id eid) override;

    private:
    View & view;
    const World & world;
  };

  class TurnSys {
    public:
    void add_turn(newcore::Id eid, unsigned int speed);
    void remove_turn(newcore::Id eid);
    void tick();

    std::pair<bool, newcore::Id> whos_turn() const;
    void finish_turn();

    private:
    struct Turn {
      newcore::Id eid;
      unsigned int energy;
      unsigned int speed;
    };

    std::map<decltype(newcore::Id::idx), Turn> turns;
  };

  class Action;

  class Engine {
    public:
    Engine(View & view)
      : view_sys(view, world)
    {}

    std::pair<bool, newcore::Id> step(unsigned int max_ticks);
    void complete_turn(const Action & action);

    newcore::Id create_hero();
    newcore::Id create_badguy(newcore::Id kill_eid);

    newcore::Id create_space(Vec2u size);
    void        destroy_space(newcore::Id sid);

    void spawn(newcore::Id eid, newcore::Id sid, Vec2i pos);
    void despawn(newcore::Id eid);

    void move_attack(newcore::Id eid, Vec2i delta);
    void wait(newcore::Id eid);

    private:
    World world;

    ViewSys view_sys;
    TurnSys turn_sys;

    void tick();
  };

  class Action {
    public:
    virtual ~Action() = default;
    virtual void perform(Engine & E, newcore::Id eid) const = 0;
  };

  class MoveAction : public Action {
    Vec2i delta;
    public:
    MoveAction(Vec2i delta) : delta(delta) {}
    void perform(Engine & E, newcore::Id eid) const override;
  };

  class WaitAction : public Action {
    public:
    void perform(Engine & E, newcore::Id eid) const override;
  };
}

#endif
