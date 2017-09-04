#ifndef RF_GAME_GAME_HPP
#define RF_GAME_GAME_HPP

#include <vector>
#include <map>
#include <deque>
#include <memory>

#include <rf/game/types.hpp>
#include <rf/game/SceneState.hpp>
#include <rf/game/Object.hpp>
#include <rf/util/Vec2.hpp>
#include <rf/util/Map.hpp>

namespace rf {
  namespace game {
    // Represents a level, should contain no game logic
    struct Level {
      Level() = default;
      Level(const Level & other) = delete;
      Level & operator=(const Level & other) = delete;
      Level(Level && other) = default;
      Level & operator=(Level && other) = default;

      Tick tick = 0;

      Map<Tile> tiles;
      std::map<Id, Object> objects;

      Id new_object_id() const;
    };

    class GameSave {
      public:
      void open();
      void close();

      // reads the world's current tick
      Tick tick() const;
      // writes the world's tick
      void set_tick(Tick t);

      // reads the player level id
      Id player_level_id() const;
      // writes the player level id
      void set_player_level_id(Id id);

      // reads the player object id
      Id player_object_id() const;
      // writes the player object id
      void set_player_object_id(Id id);

      // reads or generates a level, based on id
      Level level(Id id);
      // commits a level to the saved world
      void set_level(Id id, const Level & l);
    };

    struct PlayerMoveAction {
      Vec2i delta;
      PlayerMoveAction() {}
      PlayerMoveAction(Vec2i delta) : delta(delta) {}
    };
    struct PlayerWaitAction {
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

    class Game {
      public:
      Game(GameSave & gamesave);
      Game(const Game & other) = delete;
      Game & operator=(const Game & other) = delete;
      ~Game();

      void save() const;

      void step();
      void step(const PlayerWaitAction & action);
      void step(const PlayerMoveAction & action);

      SceneState draw(Rect2i roi);

      void handle_events(GameEventVisitor & v);
      void clear_events();

      bool is_player_turn() const;
      Tick tick() const { return level.tick; }

      private:
      GameSave & gamesave;

      Id player_level_id = 0;
      Id player_object_id = 0;

      Level level;

      std::deque<GameEvent *> events;
    };
  }
}

#endif
