#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <string>
#include <game/core.hpp>
#include <game/view.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>
#include <util/Map.hpp>

#include <game/level.hpp> // should be "private"

namespace game {
  void new_world();
  void load_world();

  // Moves the player controlled agent
  void move_attack(Vec2i delta);
  void activate_tile();

  namespace debug {
    const Agent * get_agent(Id id);
  }
}

#endif
