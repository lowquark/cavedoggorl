
#include "World.hpp"

#include <rf/game/worldgen.hpp>

#include <chrono>
#include <random>

namespace rf {
  namespace game {
    World::World() {
      seed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count();

      seed = 0;

      printf("world seed: %016lX\n", seed);
      std::mt19937_64 generator(seed);

      for(int i = 0 ; i < 10 ; i ++) {
        levels[i].seed = generator();
        printf("level %d seed: %016lX\n", i, levels[i].seed);
      }
    }
    LevelNode & World::level(Id id) {
      return levels.at(id);
    }
    const LevelNode & World::level(Id id) const {
      return levels.at(id);
    }

    Level World::render(Id id) const {
      return worldgen::troll_forest(levels.at(id).seed);
    }
    void World::step(unsigned int ticks) {
    }
  }
}
