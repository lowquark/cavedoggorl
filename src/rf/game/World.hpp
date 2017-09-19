#ifndef RF_GAME_WORLD_HPP
#define RF_GAME_WORLD_HPP

#include <map>
#include <rf/game/Level.hpp>

namespace rf {
  namespace game {
    /*
    class WorldSave {
      public:
      // reads a level, throws if it does not exist
      Level level(Id id);
      // commits a level to the saved world
      void set_level(Id id, const Level & l) nothrow;
    };
    */

    struct LevelNode {
      uint64_t seed = 0;
    };

    class World {
      public:
      World();

      LevelNode & level(Id id);
      const LevelNode & level(Id id) const;

      // render a level as a set of objects
      Level render(Id level_id) const;

      void step(unsigned int ticks);

      private:
      std::map<Id, LevelNode> levels;
      uint64_t seed = 0;
    };
  }
}

#endif
