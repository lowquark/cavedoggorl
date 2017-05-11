#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP

#include <util/Vec2.hpp>
#include <game/core.hpp>

namespace game {
  // returns the id of the player object
  Id run_until_player_turn();

  bool move_to(Id obj_id, Vec2i dstpos);
  bool move_rel(Id obj_id, Vec2i delta);
}

#endif
