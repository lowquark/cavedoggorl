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
      , player_view(view)
      , hooks(hooks)
      , engine_watch(*this)
      , engine(engine_watch) {
      load_player();
    }

    void load_player() {
      world::Id player_id = 0;

      auto world_player = world_store.player(player_id);
      auto player_entity = world_store.entity(world_player.entity);

      load_level(player_entity.location);
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

      engine.load_level(loc, new_level);

      for(auto & eid : level.entities) {
        auto e = world_store.entity(eid);
        engine.load_entity(eid, e.name, e.location, e.position);
      }
    }
    void unload_level(world::Id loc) {
    }

    void run() {
      if(engine.step(100).second) {
        while(true) {
          auto action = hooks.player_action();
          if(action) {
            engine.step(*action);
          } else {
            printf("%s: pausing\n", __PRETTY_FUNCTION__);
            break;
          }
        }
      } else {
        // no player found!
      }
    }

    private:
    class EngineWatch : public Engine::Observer {
      Game & game;

      public:
      EngineWatch(Game & game) : game(game) { printf("%s\n", __PRETTY_FUNCTION__); }

      void notify_entity_load(const Engine & engine, Id eid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
        View::EntityState es;
        es.glyph_id = engine.entity_glyph_id(eid);
        es.pos = engine.entity_position(eid);

        game.player_view.set_entity(eid, es);
      }
      void notify_entity_unload(const Engine & engine, Id eid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
        game.player_view.clear_entity(eid);
      }

      void notify_entity_move(const Engine & engine, Id eid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
        auto pos = engine.entity_position(eid);
        game.player_view.move_entity(eid, pos, pos);
      }

      void notify_level_load(const Engine & engine, Id lid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
        game.player_view.set_tiles(engine.tile_glyph_ids(lid));
      }
      void notify_level_unload(const Engine & engine, Id lid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
      }

      void notify_tile_update(const Engine & engine, Id lid, Vec2i pos) override {
        printf("%s\n", __PRETTY_FUNCTION__);
      }
    };

    world::WorldStore & world_store;
    View & player_view;

    HookHandler & hooks;
    EngineWatch engine_watch;
    Engine engine;
  };
}

#endif
