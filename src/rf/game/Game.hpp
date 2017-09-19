#ifndef RF_GAME_GAME_HPP
#define RF_GAME_GAME_HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <deque>
#include <memory>

#include <rf/game/types.hpp>
#include <rf/game/SceneState.hpp>
#include <rf/game/Object.hpp>
#include <rf/game/Tile.hpp>
#include <rf/game/Level.hpp>
#include <rf/game/World.hpp>
#include <rf/util/Vec2.hpp>
#include <rf/util/Map.hpp>
#include <rf/util/Dijkstra.hpp>
#include <rf/util/FOV.hpp>

namespace rf {
  namespace game {
    struct MissileEvent;
    struct MessageEvent;

    class DrawEventVisitor {
      public:
      virtual ~DrawEventVisitor() = default;
      virtual void visit(const MissileEvent & event) {}
      virtual void visit(const MessageEvent & event) {}
    };

    struct DrawEvent {
      virtual ~DrawEvent() = default;
      virtual void visit(DrawEventVisitor & h) = 0;
    };
    struct MissileEvent : public DrawEvent {
      Id type = 0;
      std::vector<Vec2i> path;
      // May even contain scene states (mid-explosion, for example)
      void visit(DrawEventVisitor & h) override {
        h.visit(*this);
      }
    };
    struct MessageEvent : public DrawEvent {
      std::string message;
      void visit(DrawEventVisitor & h) override {
        h.visit(*this);
      }
    };

    class Environment {
      public:
      Id player_level_id = 0;
      Id player_object_id = 0;

      Level level;

      DijkstraMap level_dijkstra;

      BresenhamFOV player_fov;

      Map<unsigned int> walk_costs;
      Map<int> player_walk_distances;
      Map<int> missile_distances;
      Map<bool> player_los;
    };

    class Game {
      public:
      Game(World & world);
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
      bool player_exists() const;

      void handle_draw_events(DrawEventVisitor & v);
      void clear_draw_events();

      private:
      World & world;
      Environment env;

      std::deque<DrawEvent *> draw_events;

      void step_environment();
      void auto_turn(Object & object);
      void wait(Object & object);
      void walk(Object & object, Vec2i delta);

      void update_player_fov();
      void update_walk_costs();
      void update_player_walk_distances();
      void update_missile_distances();
      bool is_occupied(Vec2i pos);
      void crush(Vec2i pos, int radius);

      void notify_death(Id object_id);
      void notify_move(Object & object);

      void message(const std::string & str);
    };
  }
}

#endif
