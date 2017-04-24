#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP

#include <util/Vec2.hpp>
#include <game/core.hpp>

namespace game {
  Object * get_object(Id id);
  Id create_object(const Object::Builder & builder);
  void destroy_object(Id id);
  void clear_all_objects();

  bool move_to(Id obj_id, Vec2i dstpos);
  bool move_rel(Id obj_id, Vec2i delta);
}

#endif
