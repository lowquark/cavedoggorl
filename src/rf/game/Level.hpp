#ifndef RF_GAME_LEVEL_HPP
#define RF_GAME_LEVEL_HPP

#include <map>
#include <rf/game/types.hpp>
#include <rf/game/Tile.hpp>
#include <rf/game/Object.hpp>
#include <rf/util/Map.hpp>

namespace rf {
  namespace game {
    // Represents a level, should contain no game logic (but some amount of
    // reindexing/validation logic)
    class Level {
      public:
      Level() = default;
      Level(const Level & other) = delete;
      Level & operator=(const Level & other) = delete;
      Level(Level && other) = default;
      Level & operator=(Level && other) = default;

      Tick tick = 0;

      Map<Tile> tiles;
      std::map<Id, Object> objects;

      Id new_object_id() const;
      void reindex(Object & object);
    };
  }
}

#endif
