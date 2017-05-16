#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <game/view.hpp>
#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  void add_player(unsigned int id, View * view);
  void remove_player(unsigned int id);

  void create_new();
  void save(const std::string & name);
  void load_old(const std::string & name);

  void move_attack(ObjectHandle obj, Vec2i delta);
  ObjectHandle step_until_player_turn(unsigned int max_ticks);
}

#endif
