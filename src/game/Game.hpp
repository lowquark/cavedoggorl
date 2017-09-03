#ifndef GAME_GAME_HPP
#define GAME_GAME_HPP

#include <world/world.hpp>
#include <game/Engine.hpp>

namespace game {
  class View {
    public:
    class Observer {
      public:
      virtual ~Observer() = default;
      virtual void notify_cell_update(const View & view, Vec2i pos) {}
      virtual void notify_location_name_update(const View & view) {}
      virtual void notify_focus_update(const View & view) {}
      virtual void notify_message(const View & view, const std::string & message) {}
    };

    View(Observer & observer) : observer(observer) {}

    struct Cell {
      struct Entity {
        Id game_id;
        unsigned int glyph_id;
        std::string description;

        // attributes, like moving from, fade-in/out, flashing, colored, etc
      };
      unsigned int tile_type;
      std::vector<Entity> entity;
    };
    struct Event {
      // messages, bullets, explosions, etc
    };

    const std::string & location_name() const {
      return _location_name;
    }
    void set_location_name(const std::string & str) {
      _location_name = str;
      observer.notify_location_name_update(*this);
    }

    const Map<Cell> & cells() const {
      return _cells;
    }
    void set_level_size(Vec2u size) {
      _cells.resize(size);
    }
    void set_tile(Vec2i cell_pos, unsigned int type) {
      observer.notify_cell_update(*this, cell_pos);
    }
    void remove_entity(Vec2i cell_pos, Id eid) {
      observer.notify_cell_update(*this, cell_pos);
    }
    void update_entity(Vec2i cell_pos, const Cell::Entity & e) {
      observer.notify_cell_update(*this, cell_pos);
    }

    Vec2i focus() const {
      return _focus;
    }
    void set_focus(Vec2i pos) {
      _focus = pos;
      observer.notify_focus_update(*this);
      return;
    }

    private:
    Observer & observer;

    std::string _location_name;
    Map<Cell> _cells;
    Vec2i _focus;
  };

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
        View::Cell::Entity es;
        es.game_id = eid;
        es.glyph_id = engine.entity_glyph_id(eid);
        es.description = "whoaa";

        game.player_view.update_entity(engine.entity_position(eid), es);
      }
      void notify_entity_unload(const Engine & engine, Id eid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
        //game.player_view.clear_entity(eid);
      }

      void notify_entity_move(const Engine & engine, Id eid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
        //auto pos = engine.entity_position(eid);
        //game.player_view.move_entity(eid, pos);
      }

      void notify_level_load(const Engine & engine, Id lid) override {
        printf("%s\n", __PRETTY_FUNCTION__);
        auto glyph_ids = engine.tile_glyph_ids(lid);

        game.player_view.set_level_size(glyph_ids.size());

        for(unsigned int j = 0 ; j < glyph_ids.size().y ; j ++) {
          for(unsigned int i = 0 ; i < glyph_ids.size().x ; i ++) {
            auto pos = Vec2i((int)i, (int)j);
            game.player_view.set_tile(pos, glyph_ids.get(pos));
          }
        }
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
