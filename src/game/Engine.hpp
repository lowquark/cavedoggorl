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

  class Engine {
    public:
    class Observer {
      public:
      virtual ~Observer() = default;

      virtual void   notify_entity_load(const Engine & eng, Id eid) {}
      virtual void notify_entity_unload(const Engine & eng, Id eid) {}

      virtual void   notify_entity_move(const Engine & eng, Id eid) {}

      virtual void    notify_level_load(const Engine & eng, Id lid) {}
      virtual void  notify_level_unload(const Engine & eng, Id lid) {}

      virtual void   notify_tile_update(const Engine & eng, Id lid, Vec2i pos) {}
    };

    Engine(Observer & obs) : obs(obs) {}

    void load_entity(Id eid, const std::string & name, Id loc, Vec2i pos);
    void unload_entity(Id eid);

    void load_level(Id loc, const Level & level);
    void unload_level(Id loc);

    void perform(const Action & action, Id eid);
    void perform(const MoveAction & action, Id eid);
    void perform(const WaitAction & action, Id eid);
    void perform(const StairAction & action, Id eid);

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

    Vec2i entity_position(Id eid) const;
    Id entity_location(Id eid) const;
    unsigned int entity_glyph_id(Id eid) const;

    bool level_is_loaded(Id lid) const;
    Vec2u level_size(Id lid) const;
    unsigned int tile_glyph_id(Id lid, Vec2i pos) const;
    Map<unsigned int> tile_glyph_ids(Id lid) const;

    private:
    World world;

    Observer & obs;
    TurnSys turn_sys;

    void complete_turn(const Action & action);
    void tick();
  };
}

#endif
