#ifndef GAME_GAME_HPP
#define GAME_GAME_HPP

#include <set>
#include <memory>

#include <game/actions.hpp>
#include <game/World.hpp>
#include <game/view.hpp>
#include <game/FOV.hpp>
#include <game/Color.hpp>

namespace game {
  class View {
    public:
    virtual ~View() = default;

    struct TileState {
      unsigned int glyph_id = 0;
      bool visible = true;
    };
    struct EntityState {
      Vec2i pos;
      unsigned int glyph_id = 0;
    };


    const std::string & location_name() const { return _location_name; }
    const Map<TileState> & tiles() const { return _tiles; }
    const std::map<unsigned int, EntityState> & entities() const { return _entities; }
    const std::pair<bool, unsigned int> & player_entity_id() const { return _player_entity_id; }

    void set_location_name(const std::string & str) {
      _location_name = str;
      notify_location_name_update();
    }
    void set_tiles(const Map<TileState> & tiles) {
      _tiles = tiles;
      notify_tiles_update();
    }
    void set_entity(unsigned int id, const EntityState & state) {
      _entities[id] = state;
      notify_entity_update(id);
    }

    void move_entity(unsigned int id, Vec2i from, Vec2i to) {
      auto kvpair_it = _entities.find(id);
      if(kvpair_it != _entities.end()) {
        auto & e = kvpair_it->second;
        e.pos = to;
        notify_entity_move(id, from, to);
      }
    }

    private:
    std::string _location_name;
    Map<TileState> _tiles;
    std::map<unsigned int, EntityState> _entities;
    std::pair<bool, unsigned int> _player_entity_id = decltype(_player_entity_id)(false, 0);


    virtual void notify_tiles_update() {}

    virtual void notify_entity_update(unsigned int id) {};
    virtual void notify_entity_delete(unsigned int id) {}
    virtual void notify_entity_show(unsigned int id) {}
    virtual void notify_entity_hide(unsigned int id) {}
    // Could probably be a little more creative with deducing from/to
    virtual void notify_entity_move(unsigned int id, Vec2i from, Vec2i to) {}
    virtual void notify_player_update() {}
    virtual void notify_fov_update() {}

    virtual void notify_location_name_update() {}
    virtual void notify_message(const std::string & message) {}
  };

  class Player {
    public:
    Player(WorldId id, View & view) : _id(id), _view(view) {}

    WorldId id() const { return _id; }
    View & view() const { return _view; }

    std::unique_ptr<Action> next_action;

    private:
    WorldId _id;
    View & _view;
  };

  class ViewSys {
    public:
    ViewSys(std::vector<Player *> & players, const World & world) : players(players), world(world) {}

    void on_spawn(WorldId eid);
    void on_move(WorldId eid, Vec2i from, Vec2i to);

    void load_level(Player * player);

    private:
    std::vector<Player *> & players;
    const World & world;
  };

  class TurnSys {
    public:
    void add_turn(WorldId eid, unsigned int speed);
    void remove_turn(WorldId eid);
    void tick();

    std::pair<bool, WorldId> whos_turn() const;
    void finish_turn();

    private:
    struct Turn {
      unsigned int energy;
      unsigned int speed;
    };

    std::map<WorldId, Turn> turns;
  };

  class MobSys {
    public:
    MobSys(ViewSys & view_sys, World & world)
      : view_sys(view_sys)
      , world(world)
    {}

    void move_attack(WorldId eid, Vec2i delta);

    private:
    ViewSys & view_sys;
    World & world;
  };

  class Engine {
    public:
    Engine()
      : view_sys(players, world)
      , mob_sys(view_sys, world)
    {
      world.player_locations[0] = 0;
    }

    Player * create_player(WorldId id, View & view);

    std::pair<bool, WorldId> step(unsigned int max_ticks);
    void complete_turn(const Action & action);

    /*
    WorldId load_entity(const std::string & type, const std::string & space_name, Vec2i pos);
    void   unload_entity(WorldId id);

    WorldId load_space(const std::string & name, Vec2u size);
    void unload_space(WorldId id);

    void spawn_entity(WorldId eid, Vec2i pos, WorldId sid);
    void despawn_entity(WorldId eid);
    */

    private:
    std::vector<Player *> players;
    World world;

    ViewSys view_sys;
    MobSys mob_sys;

    TurnSys turn_sys;

    void spawn_player(Player * player);

    Space & load_level(WorldId id);
    void unload_level(WorldId id);

    void tick();
  };
}

#endif
