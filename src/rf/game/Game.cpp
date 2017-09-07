
#include "Game.hpp"

#include <cassert>

namespace rf {
  namespace game {
    void GameSave::open() {
      close();
    }
    void GameSave::close() {
    }

    Id GameSave::player_level_id() const {
      return 0;
    }
    void GameSave::set_player_level_id(Id id) {
    }

    Id GameSave::player_object_id() const {
      return 0;
    }
    void GameSave::set_player_object_id(Id id) {
    }

    Level GameSave::level(Id id) {
      assert(id == 0);

      Vec2u level_size(32, 32);

      Level lv;
      lv.tiles.resize(level_size);

      srand(time(NULL));

      lv.objects[0].add(new BasicObjectGlyph(Glyph(3, Color(0xFF, 0xCC, 0x99))));
      lv.objects[1].add(new BasicObjectGlyph(Glyph(0, Color(0xFF, 0xCC, 0x99))));
      lv.objects[2].add(new BasicObjectGlyph(Glyph(1, Color(0xFF, 0xCC, 0x99))));
      lv.objects[3].add(new BasicObjectGlyph(Glyph(2, Color(0xFF, 0xCC, 0x99))));

      lv.objects[0].set_pos(Vec2i(5, 5));
      lv.objects[1].set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));
      lv.objects[2].set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));
      lv.objects[3].set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));

      for(unsigned int j = 0 ; j < level_size.y ; j ++) {
        for(unsigned int i = 0 ; i < level_size.x ; i ++) {
          Vec2u pi(i, j);
          auto & tile = lv.tiles.get(pi);
          unsigned int r = (rand() % 100);
          if(r <= 8) {
            tile.add(new BasicTileGlyph(Glyph(5 + 7*16, Color(0x33, 0x66, 0x33))));
          } else {
            tile.add(new BasicTileGlyph(Glyph(5 + 6*16, Color(0x22, 0x44, 0x44))));
          }
        }
      }

      return lv;
    }
    void GameSave::set_level(Id id, const Level & l) {
    }

    Game::Game(GameSave & gamesave)
      : gamesave(gamesave) {
      player_level_id = gamesave.player_level_id();
      player_object_id = gamesave.player_object_id();

      level = gamesave.level(player_level_id);
    }
    Game::~Game() {
      save();
      clear_events();
    }

    void Game::save() const {
      gamesave.set_player_level_id(player_level_id);
      gamesave.set_player_object_id(player_object_id);

      gamesave.set_level(player_level_id, level);
    }

    void Game::step() {
      // object turn, or step environment

      for(auto & kvpair : level.objects) {
        auto object_id = kvpair.first;
        auto & object = kvpair.second;

        if(object_id != player_object_id) {
          walk(object, Vec2i((rand() % 3) - 1, (rand() % 3) - 1));
        }
      }
    }
    void Game::step(const PlayerWaitAction & action) {
      step();
    }
    void Game::step(const PlayerMoveAction & action) {
      auto kvpair_it = level.objects.find(player_object_id);
      if(kvpair_it != level.objects.end()) {
        auto & player_object = kvpair_it->second;
        walk(player_object, action.delta);
      }

      step();
    }

    SceneState Game::draw(Rect2i roi) {
      SceneState st;

      st.cells.resize(roi.size);

      for(unsigned int j = 0 ; j < roi.size.y ; j ++) {
        for(unsigned int i = 0 ; i < roi.size.x ; i ++) {
          Vec2u pi(i, j);
          Vec2i p(pi + roi.pos);

          auto & cell = st.cells.get(pi);

          if(p.x >= 0 && p.y >= 0 && level.tiles.valid(Vec2u(p))) {
            auto & tile = level.tiles.get(p);
            cell.tile.glyph = tile.glyph();
          } else {
            cell.tile.glyph = Glyph(0, Color());
          }
        }
      }

      for(auto & kvpair : level.objects) {
        auto id = kvpair.first;
        auto & o = kvpair.second;

        // check to make sure the object will fit on screen
        Vec2i pi = o.pos() - roi.pos;

        if(pi.x >= 0 && pi.y >= 0 && st.cells.valid(Vec2u(pi))) {
          auto & cell = st.cells.get(pi);
          cell.objects.emplace_back();
          cell.objects.back().glyph = o.glyph();
          cell.objects.back().object_id = id;
        }
      }

      return st;
    }

    void Game::handle_events(GameEventVisitor & v) {
      if(events.size()) {
        auto events_copy = std::move(events);
        for(auto & ev : events_copy) {
          ev->visit(v);
        }
        while(events_copy.size()) {
          delete events_copy.front();
          events_copy.pop_front();
        }
      }
    }
    void Game::clear_events() {
      while(events.size()) {
        delete events.front();
        events.pop_front();
      }
    }

    bool Game::is_player_turn() const {
      return true;
    }

    void Game::walk(Object & object, Vec2i delta) {
      Vec2i destination = object.pos() + delta;
      if(destination.x >= 0 && destination.x < level.tiles.size().x &&
         destination.y >= 0 && destination.y < level.tiles.size().y) {
        object.set_pos(destination);
      }
    }
  }
}

