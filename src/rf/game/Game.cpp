
#include "Game.hpp"

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
      Level lv;
      lv.tiles.resize(Vec2u(32, 32));

      lv.objects[0].add(new BasicObjectGlyph(Glyph(3, Color(0xFF, 0xCC, 0x99))));

      for(unsigned int j = 0 ; j < 32 ; j ++) {
        for(unsigned int i = 0 ; i < 32 ; i ++) {
          Vec2u pi(i, j);
          auto & tile = lv.tiles.get(pi);
          unsigned int r = (rand() % 100);
          if(r == 0) {
            tile.add(new BasicTileGlyph(Glyph(0, Color(0xFF, 0xCC, 0x99))));
          } else if(r <= 8) {
            tile.add(new BasicTileGlyph(Glyph(5 + 4*16, Color(0x33, 0x66, 0x33))));
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
    }
    void Game::step(const PlayerWaitAction & action) {
      step();
    }
    void Game::step(const PlayerMoveAction & action) {
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
  }
}

