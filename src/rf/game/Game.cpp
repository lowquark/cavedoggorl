
#include "Game.hpp"

#include <cassert>
#include <limits>

namespace rf {
  namespace game {
    Game::Game(GameSave & gamesave)
      : gamesave(gamesave) {
      player_level_id = gamesave.player_level_id();
      player_object_id = gamesave.player_object_id();

      level = gamesave.level(player_level_id);

      walk_costs.resize(level.tiles.size());
      walk_costs.fill(1);
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

      if(turn_queue.size()) {
        Id object_id = turn_queue.front();
        Object & object = level.objects.at(object_id);

        assert(object_id != 0);

        if(object_id != player_object_id) {
          // ai turn
          ai_turn(object);
        } else {
          // player turn, default to wait
          wait(object);
        }

        if(object.turn_energy() < 0) {
          turn_queue.pop_front();
        }
      } else {
        // environment turn
      }
    }
    void Game::step(const ObjectWait & action) {
      // player turn, if it is indeed the player's turn
      // otherwise, step the world normally
      if(turn_queue.size()) {
        Id object_id = turn_queue.front();

        if(object_id == player_object_id) {
          Object & object = level.objects.at(object_id);

          wait(object);

          if(object.turn_energy() < 0) {
            turn_queue.pop_front();
          }
        } else {
          // step other
          step();
        }
      } else {
        // step other
        step();
      }
    }
    void Game::step(const ObjectMove & action) {
      if(turn_queue.size() && turn_queue.front() == player_object_id) {
        Object & player_object = level.objects.at(turn_queue.front());
        walk(player_object, action.delta);

        player_walk_distance_dijkstra.compute(player_walk_distances, walk_costs, player_object.pos());

        if(player_object.turn_energy() < 0) {
          turn_queue.pop_front();
        }
      }
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

    bool Game::is_object_turn() const {
      return !turn_queue.empty();
    }
    bool Game::is_environment_turn() const {
      return turn_queue.empty();
    }

    bool Game::is_player_turn() const {
      if(turn_queue.empty()) {
        return false;
      } else {
        return player_object_id && turn_queue.front() == player_object_id;
      }
    }

    void Game::ai_turn(Object & object) {
      std::vector<Vec2i> min_deltas;
      int min_distance = DijkstraMap::infinity;
      for(int y = -1 ; y <= 1 ; y ++) {
        for(int x = -1 ; x <= 1 ; x ++) {
          if(!(x == 0 && y == 0)) {
            Vec2i delta(x, y);
            Vec2u pos = object.pos() + delta;
            if(player_walk_distances.valid(pos)) {
              unsigned int distance = player_walk_distances[pos];
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
      }
    }
    void Game::wait(Object & object) {
      object.add_turn_energy(10);
    }
    void Game::walk(Object & object, Vec2i delta) {
      Vec2i destination = object.pos() + delta;
      if(destination.x >= 0 && destination.x < level.tiles.size().x &&
         destination.y >= 0 && destination.y < level.tiles.size().y) {
        object.set_pos(destination);
      }
      object.add_turn_energy(10);
    }
  }
}

