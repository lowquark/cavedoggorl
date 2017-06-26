#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include <game/actions.hpp>
#include <game/World.hpp>
#include <game/view.hpp>
#include <game/FOV.hpp>
#include <game/Color.hpp>

// game: loaded, volatile, fun state
namespace game {
  typedef unsigned int Id;

  class ViewSys {
    public:
    ViewSys(const World & world)
      : world(world) {}

    void notify_spawn(Id eid);
    void notify_despawn(Id eid);
    void notify_move(Id eid, Vec2i from, Vec2i to);
    void notify_fov_update(Id eid);

    void set_view(Id eid, View & view);
    void unset_view(Id eid);

    private:
    const World & world;

    std::map<Id, std::reference_wrapper<View>> views;

    void full_update(View & view, const Level & level);
  };

  class FOVSys {
    public:
    FOVSys(World & world)
      : world(world) {}

    void add(Id eid);
    void remove(Id eid);
    void notify_move(Id eid, Vec2i from, Vec2i to);

    private:
    World & world;

    std::set<Id> entities;
  };

  class TurnSys {
    public:
    void add_turn(Id eid, unsigned int speed);
    void remove_turn(Id eid);
    void tick();

    std::pair<bool, Id> whos_turn() const;
    void finish_turn();

    private:
    struct Turn {
      unsigned int energy;
      unsigned int speed;
    };

    std::map<Id, Turn> turns;
  };

  class MobSys {
    public:
    MobSys(World & world, ViewSys & view_sys)
      : world(world)
      , view_sys(view_sys) {}

    void perform(Id eid, const MoveAction & action);
    void perform(Id eid, const WaitAction & action);
    void perform(Id eid, const StairAction & action);

    private:
    World & world;
    ViewSys & view_sys;
  };

  class EngineObserver {
    public:
    virtual ~EngineObserver() = default;
    virtual void notify_spawn(Id eid) {}
    virtual void notify_despawn(Id eid) {}
    virtual void notify_move(Id eid) {}
    virtual void notify_fov_update(Id eid) {}
  };
  class Engine {
    public:
    Engine(EngineObserver & obs)
      : obs(obs)
      , fov_sys(world)
      , view_sys(world)
      , mob_sys(world, view_sys) {}

    void load_entity(Id eid, const std::string & name, Id loc, Vec2i pos);
    void unload_entity(Id eid);

    void load_level(Id loc, const Level & level);
    void unload_level(Id loc);

    void attach_player(Id pid, Id eid, View & view);

    // Run until external input is required
    // Returns the mob requiring input's id
    Id step();
    // Runs until external input is required, or max_ticks have elapsed
    // If external input is required, returns (mob_id, true), where mob_id is the id of the mob requiring input
    // If max_ticks have been elapsed, returns (0, false)
    std::pair<Id, bool> step(unsigned int max_ticks);
    // If external input is required, executes the input action. Continues to run until input is required.
    // Returns the mob requiring input's id
    Id step(const Action & action);
    // If external input is required, executes the input action.
    // Runs until external input is required, or max_ticks have elapsed
    // If external input is required, returns (mob_id, true), where mob_id is the id of the mob requiring input
    // If max_ticks have been elapsed, returns (0, false)
    std::pair<Id, bool> step(const Action & action, unsigned int max_ticks);

    private:
    World world;

    EngineObserver obs;
    FOVSys fov_sys;
    ViewSys view_sys;
    MobSys mob_sys;
    TurnSys turn_sys;

    void complete_turn(const Action & action);
    void tick();
  };
}

#endif
