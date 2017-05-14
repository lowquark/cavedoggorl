#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <game/view.hpp>
#include <util/Vec2.hpp>

namespace game {
  void create_new();
  void save(const std::string & name);
  void load_old(const std::string & name);

  void set_view(View & view);
  View & view();

  // Moves the player controlled agent, steps the game
  void move_attack(Vec2i delta);
  // Causes the player controlled agent to activate a nearby tile, steps the game
  void activate_tile();
}

#endif
