#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include <game/actions.hpp>
#include <game/World.hpp>
#include <game/view.hpp>
#include <game/FOV.hpp>
#include <game/Color.hpp>

#include <world/world.hpp>

// game: loaded, volatile, fun state
namespace game {
  class ViewSys {
    public:
    ViewSys(const World & world)
      : world(world) {}

    void on_spawn(world::Id eid);
    void on_despawn(world::Id eid);
    void on_move(world::Id eid, Vec2i from, Vec2i to);

    void set_view(world::Id pid, View & view);
    void unset_view(world::Id pid);

    void on_transfer(world::Id eid, world::Id loc);

    private:
    const World & world;

    struct ViewProxy {
      View & view;
      ViewProxy(View & view) : view(view) {}
    };
    std::map<world::Id, ViewProxy> view_proxies;

    void full_update(View & view, const Level & level);
  };

  class FOVSys {
    public:
    FOVSys(const World & world)
      : world(world) {}

    void on_spawn(world::Id eid);
    void on_despawn(world::Id eid);
    void on_move(world::Id eid, Vec2i from, Vec2i to);

    private:
    const World & world;
  };

  class TurnSys {
    public:
    void add_turn(world::Id eid, unsigned int speed);
    void remove_turn(world::Id eid);
    void tick();

    std::pair<bool, world::Id> whos_turn() const;
    void finish_turn();

    private:
    struct Turn {
      unsigned int energy;
      unsigned int speed;
    };

    std::map<world::Id, Turn> turns;
  };

  class RoamSys {
    public:
    RoamSys(World & world, world::WorldStore & world_store)
      : world(world)
      , world_store(world_store)
    {}

    void tick();

    private:
    World & world;
    world::WorldStore & world_store;
  };

  class LoadSys {
    public:
    LoadSys(World & world,
            world::WorldStore & world_store,
            ViewSys & view_sys,
            TurnSys & turn_sys)
      : world(world)
      , world_store(world_store)
      , view_sys(view_sys)
      , turn_sys(turn_sys)
    {}

    bool load_player(world::Id pid);
    void unload_player(world::Id pid);
    void transfer_mob(world::Id eid, world::Id loc, Vec2i pos);

    private:
    World & world;
    world::WorldStore & world_store;
    ViewSys & view_sys;
    TurnSys & turn_sys;

    void load_entity(world::Id eid, const world::Entity & state);
    void unload_entity(world::Id eid);

    Level & load_level(world::Id world_id);
    void unload_level(world::Id world_id);
  };

  class MobSys {
    public:
    MobSys(World & world, LoadSys & load_sys, ViewSys & view_sys)
      : world(world)
      , view_sys(view_sys)
      , load_sys(load_sys) {}

    void perform(world::Id eid, const MoveAction & action);
    void perform(world::Id eid, const WaitAction & action);
    void perform(world::Id eid, const StairAction & action);

    private:
    World & world;
    ViewSys & view_sys;
    LoadSys & load_sys;
  };

  class Engine {
    public:
    typedef unsigned int Id;

    class HookHandler {
      public:
      virtual ~HookHandler() = default;
      virtual void on_player_turn(Engine::Id player_id) {}
      virtual void on_transfer(Engine::Id eid, world::Id wid, Vec2i pos) {}
    };

    Engine(HookHandler & hh, world::WorldStore & world_store)
      : hh(hh)
      , load_sys(world, world_store, view_sys, turn_sys)
      , roam_sys(world, world_store)
      , fov_sys(world)
      , view_sys(world)
      , mob_sys(world, load_sys, view_sys) {}

    // [Turn Sys] -- action       --> [Mob Sys]
    // [Mob Sys]  -- move_request --> [Phys Sys]
    // [Phys Sys] -- move_event   --> [FOV Sys]
    // [Phys Sys] -- move_event   --> [View Sys]
    // [FOV Sys]  -- fov_update   --> [View Sys]

    void spawn_player(world::Id pid, View & view);
    void despawn_player(world::Id pid);

    bool run(unsigned int max_ticks);
    void run();

    void complete_turn(const Action & action);

    bool paused() const { return _paused; }
    void set_paused(bool paused) { this->_paused = paused; }

    private:
    HookHandler & hh;
    World world;

    LoadSys load_sys;
    RoamSys roam_sys;
    FOVSys fov_sys;
    ViewSys view_sys;
    MobSys mob_sys;
    TurnSys turn_sys;

    bool _paused = false;
    void tick();
  };
}

#endif
