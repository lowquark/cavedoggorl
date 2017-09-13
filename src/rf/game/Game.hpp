#ifndef RF_GAME_GAME_HPP
#define RF_GAME_GAME_HPP

#include <vector>
#include <map>
#include <deque>
#include <memory>

#include <rf/game/types.hpp>
#include <rf/game/SceneState.hpp>
#include <rf/game/Object.hpp>
#include <rf/game/Tile.hpp>
#include <rf/game/GameSave.hpp>
#include <rf/util/Vec2.hpp>
#include <rf/util/Map.hpp>
#include <rf/util/Dijkstra.hpp>

namespace rf {
  namespace game {
    struct ObjectMove {
      Vec2i delta;
      ObjectMove() {}
      ObjectMove(Vec2i delta) : delta(delta) {}
    };
    struct ObjectWait {
    };

    struct MissileEvent;

    class GameEventVisitor {
      public:
      virtual ~GameEventVisitor() = default;
      virtual void visit(const MissileEvent & event) {}
    };

    struct GameEvent {
      virtual ~GameEvent() = default;
      virtual void visit(GameEventVisitor & h) = 0;
    };
    struct MissileEvent : public GameEvent {
      Id type;
      std::vector<Vec2i> path;
      // May even contain scene states (mid-explosion, for example)
      void visit(GameEventVisitor & h) override {
        h.visit(*this);
      }
    };

    class Environment {
      public:
      Id player_level_id = 0;
      Id player_object_id = 0;

      Level level;

      Map<unsigned int> walk_costs;
      Map<int> player_walk_distances;
      DijkstraMap player_walk_distance_dijkstra;
    };

    class Game {
      public:
      Game(GameSave & gamesave);
      Game(const Game & other) = delete;
      Game & operator=(const Game & other) = delete;
      ~Game();

      void save() const;

      SceneState draw(Rect2i roi) const;

      void step();
      void wait();
      void move(Vec2i delta);

      Id next_object_turn() const;
      bool is_player_turn() const;

      void handle_events(GameEventVisitor & v);
      void clear_events();

      private:
      GameSave & gamesave;
      Environment env;

      std::deque<GameEvent *> events;

      void step_environment();
      void auto_turn(Object & object);
      void walk(Object & object, Vec2i delta);
      void wait(Object & object);

      void on_spawn(Object & object);
      void on_death(Object & object);
      void on_update_position(Object & object);
    };
  }
}

#endif
