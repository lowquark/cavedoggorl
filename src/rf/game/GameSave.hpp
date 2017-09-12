#ifndef RF_GAME_GAMESAVE_HPP
#define RF_GAME_GAMESAVE_HPP

#include <cstdint>
#include <map>
#include <rf/game/types.hpp>
#include <rf/game/Tile.hpp>
#include <rf/game/Object.hpp>
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
  }
}

#endif
