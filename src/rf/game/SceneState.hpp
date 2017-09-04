#ifndef RF_GAME_SCENESTATE_HPP
#define RF_GAME_SCENESTATE_HPP

#include <string>
#include <vector>
#include <map>

#include <rf/game/types.hpp>
#include <rf/game/Glyph.hpp>
#include <rf/util/Map.hpp>

namespace rf {
  namespace game {
    // Representation of what the game world 'looks like' to the player
    struct SceneState {
      struct Cell {
        struct Tile {
          // Renderable
          Glyph glyph;
          std::string description;
        };

        struct Object {
          // Renderable
          Glyph glyph;
          std::string description;

          // For interfacing with this object
          Id object_id;

          // attributes, like moving from, fade-in/out, flashing, colored, etc
        };

        Tile tile;

        // May contain a stack of objects
        std::vector<Object> objects;
      };

      Map<Cell> cells;
    };
  }
}

#endif
