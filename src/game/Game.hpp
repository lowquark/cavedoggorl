#ifndef GAME_GAME_HPP
#define GAME_GAME_HPP

#include <world/world.hpp>
#include <game/Engine.hpp>

namespace game {
  // Single player game
  class Game {
    public:
    class HookHandler {
      public:
      virtual ~HookHandler() = default;
      virtual std::unique_ptr<Action> player_action() { return nullptr; }
    };

    Game(HookHandler & hooks, View & view, world::WorldStore & world_store)
      : world_store(world_store)
      , hooks(hooks)
      , eng(ew) {
      load_player(view);
    }

    void load_player(View & view) {
      world::Id player_id = 0;

      auto world_player = world_store.player(player_id);
      auto player_entity = world_store.entity(world_player.entity);

      load_level(player_entity.location);
      eng.attach_player(player_id, world_player.entity, view);
    }

    void load_level(world::Id loc) {
      world::Level level = world_store.level(loc);

      game::Level new_level;
      new_level.tiles.resize(level.tiles.size());
      new_level.opaque.resize(level.tiles.size());
      for(unsigned int j = 0 ; j < level.tiles.size().y ; j ++) {
        for(unsigned int i = 0 ; i < level.tiles.size().x ; i ++) {
          Vec2i pos(i, j);

          if(level.tiles.get(pos).type_id == 1) {
            new_level.tiles.set(pos, 1);
            new_level.opaque.set(pos, 1);
          } else if(level.tiles.get(pos).type_id == 2) {
            new_level.tiles.set(pos, 2);
            new_level.opaque.set(pos, 0);
          }
        }
      }

      eng.load_level(loc, new_level);

      for(auto & eid : level.entities) {
        auto e = world_store.entity(eid);
        eng.load_entity(eid, e.name, e.location, e.position);
      }
    }
    void unload_level(world::Id loc) {
    }

    void run() {
      if(eng.step(100).second) {
        while(true) {
          auto action = hooks.player_action();
          if(action) {
            eng.step(*action);
          } else {
            printf("%s: pausing\n", __PRETTY_FUNCTION__);
            break;
          }
        }
      } else {
        // no player found!
      }
    }

    bool paused() const { return _paused; }
    void set_paused(bool paused) { this->_paused = paused; }

    private:
    class EngineWatch : public EngineObserver {
      virtual void notify_spawn(Id eid) {}
      virtual void notify_despawn(Id eid) {}
      virtual void notify_move(Id eid) {}
      virtual void notify_fov_update(Id eid) {}
    };

    bool _paused = false;

    world::WorldStore & world_store;

    HookHandler & hooks;
    EngineWatch ew;
    Engine eng;
  };
}

#endif
