
#include "Game.hpp"

#include <cassert>
#include <limits>

namespace rf {
  namespace game {
    Game::Game(GameSave & gamesave)
      : gamesave(gamesave) {
      env.player_level_id = gamesave.player_level_id();
      env.player_object_id = gamesave.player_object_id();

      env.level = gamesave.level(env.player_level_id);

      env.walk_costs.resize(env.level.tiles.size());
      env.walk_costs.fill(1);
    }
    Game::~Game() {
      clear_draw_events();
    }

    void Game::save() const {
      gamesave.set_player_level_id(env.player_level_id);
      gamesave.set_player_object_id(env.player_object_id);

      gamesave.set_level(env.player_level_id, env.level);
    }

    SceneState Game::draw(Rect2i roi) const {
      SceneState st;

      st.cells.resize(roi.size);

      for(unsigned int j = 0 ; j < roi.size.y ; j ++) {
        for(unsigned int i = 0 ; i < roi.size.x ; i ++) {
          Vec2u pi(i, j);
          Vec2i p(pi + roi.pos);

          auto & cell = st.cells.get(pi);

          if(p.x >= 0 && p.y >= 0 && env.level.tiles.valid(Vec2u(p))) {
            auto & tile = env.level.tiles.get(p);
            cell.tile.glyph = tile.glyph();
          } else {
            cell.tile.glyph = Glyph(0, Color());
          }
        }
      }

      for(auto & kvpair : env.level.objects) {
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

    void Game::step() {
      Id object_id = next_object_turn();

      if(object_id == 0) {
        step_environment();
      } else {
        Object & object = env.level.objects.at(object_id);
        auto_turn(object);
      }
    }
    void Game::wait() {
      Id object_id = next_object_turn();

      if(object_id == 0) {
        step_environment();
      } else {
        Object & object = env.level.objects.at(object_id);
        if(object.playable()) {
          wait(object);
        } else {
          auto_turn(object);
        }
      }
    }
    void Game::move(Vec2i delta) {
      Id object_id = next_object_turn();

      if(object_id == 0) {
        step_environment();
      } else {
        Object & object = env.level.objects.at(object_id);
        if(object.playable()) {
          walk(object, delta);
        } else {
          auto_turn(object);
        }
      }
    }

    Id Game::next_object_turn() const {
      for(auto & kvpair : env.level.objects) {
        auto & id = kvpair.first;
        auto & object = kvpair.second;
        if(object.has_turn() && object.turn_energy() > 0) {
          return id;
        }
      }
      return 0;
    }
    bool Game::is_player_turn() const {
      Id turn_id = next_object_turn();
      return turn_id && turn_id == env.player_object_id;
    }
    bool Game::player_exists() const {
      return env.player_object_id != 0;
    }

    void Game::handle_draw_events(DrawEventVisitor & v) {
      if(draw_events.size()) {
        auto draw_events_copy = std::move(draw_events);
        for(auto & ev : draw_events_copy) {
          ev->visit(v);
        }
        while(draw_events_copy.size()) {
          delete draw_events_copy.front();
          draw_events_copy.pop_front();
        }
      }
    }
    void Game::clear_draw_events() {
      while(draw_events.size()) {
        delete draw_events.front();
        draw_events.pop_front();
      }
    }

    void Game::step_environment() {
      for(auto & kvpair : env.level.objects) {
        auto & o = kvpair.second;
        o.add_turn_energy(10);
      }
    }
    void Game::auto_turn(Object & object) {
      std::vector<Vec2i> min_deltas;
      int min_distance = DijkstraMap::infinity;
      for(int y = -1 ; y <= 1 ; y ++) {
        for(int x = -1 ; x <= 1 ; x ++) {
          if(!(x == 0 && y == 0)) {
            Vec2i delta(x, y);
            Vec2u pos = object.pos() + delta;
            if(env.player_walk_distances.valid(pos)) {
              unsigned int distance = env.player_walk_distances[pos];
              if(distance < min_distance) {
                min_deltas.clear();
                min_deltas.push_back(delta);
                min_distance = distance;
              } else if(distance == min_distance) {
                min_deltas.push_back(delta);
              }
            }
          }
        }
      }
      if(min_deltas.size()) {
        walk(object, min_deltas[rand() % min_deltas.size()]);
      } else {
        walk(object, Vec2i((rand() % 3) - 1, (rand() % 3) - 1));
      }
    }

    static Vec2i select_dir8(unsigned int x) {
      switch(x) {
        default:
          return Vec2i(0, 0);
        case 0:
          return Vec2i(1, 0);
        case 1:
          return Vec2i(1, 1);
        case 2:
          return Vec2i(0, 1);
        case 3:
          return Vec2i(-1, 1);
        case 4:
          return Vec2i(-1, 0);
        case 5:
          return Vec2i(-1, -1);
        case 6:
          return Vec2i(0, -1);
        case 7:
          return Vec2i(1, -1);
      }
    }

    void Game::wait(Object & object) {
      object.use_turn_energy(10);

      auto pos = object.pos();
      std::vector<Vec2i> path;
      Vec2i dir = select_dir8(rand() % 8);

      unsigned int length = (rand() % 2) + 8;
      for(unsigned int i = 0 ; i < length ; i ++) {
        path.push_back(pos);
        pos += dir;
        if((rand() % 3) == 0) {
          dir = select_dir8(rand() % 8);
        }
      }

      crush(path.back(), 1);

      MissileEvent * ev = new MissileEvent;
      ev->path = path;
      draw_events.push_back(ev);
    }
    void Game::walk(Object & object, Vec2i delta) {
      Vec2i destination = object.pos() + delta;
      if(destination.x >= 0 && destination.x < env.level.tiles.size().x &&
         destination.y >= 0 && destination.y < env.level.tiles.size().y) {
        object.set_pos(destination);
        notify_move(object);
      }
      object.use_turn_energy(10);
    }

    void Game::crush(Vec2i pos, unsigned int radius) {
      std::vector<Id> kill_list;
      for(auto & kvpair : env.level.objects) {
        auto id = kvpair.first;
        auto & o = kvpair.second;
        if(o.pos().x < pos.x - radius) { continue; }
        if(o.pos().x > pos.x + radius) { continue; }
        if(o.pos().y < pos.y - radius) { continue; }
        if(o.pos().y > pos.y + radius) { continue; }
        kill_list.push_back(id);
      }

      for(auto & id : kill_list) {
        env.level.objects.erase(id);
        notify_death(id);
      }
    }

    void Game::notify_death(Id object_id) {
      if(object_id == env.player_object_id) {
        env.player_object_id = 0;
      }

      // recompute dijkstra maps based on goals
    }
    void Game::notify_move(Object & object) {
      // recompute dijkstra maps based on goals
    }
  }
}

