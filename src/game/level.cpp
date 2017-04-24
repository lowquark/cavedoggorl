
#include "level.hpp"

#include <game/view.hpp>
#include <game/events.hpp>
#include <game/properties.hpp>
#include <util/serial.hpp>

namespace game {
  std::vector<Object *> all_objects;


  Object * get_object(Id id) {
    if(id == 0) {
      return nullptr;
    }
    if(id > all_objects.size()) {
      return nullptr;
    }

    return all_objects[id - 1];
  }
  Id create_object(const Object::Builder & builder) {
    for(std::size_t idx = 0 ; idx < all_objects.size() ; idx ++) {
      auto o = all_objects[idx];
      if(o == nullptr) {
        o = new Object(builder.build());
        return idx + 1;
      }
    }
    all_objects.push_back(new Object(builder.build()));
    return all_objects.size();
  }
  void destroy_object(Id id) {
    if(id == 0) {
      return;
    }
    if(id > all_objects.size()) {
      return;
    }

    auto & object = all_objects[id - 1];

    if(object) {
      delete object;
      object = nullptr;
    }
  }
  void clear_all_objects() {
    for(auto & o : all_objects) {
      delete o;
    }
    all_objects.clear();
  }


  bool move_to(Id obj_id, Vec2i dstpos) {
    auto obj = get_object(obj_id);
    if(obj) {
      for(auto & other_obj : all_objects) {
        if(other_obj && other_obj != obj) {
          GetPositionEvent event;
          other_obj->fire_event(event);

          if(event.pos == dstpos) {
            return false;
          }
        }
      }

      GetPositionEvent get_pos;
      obj->fire_event(get_pos);

      SetPositionEvent set_pos(dstpos);
      obj->fire_event(set_pos);

      view().on_agent_move(obj_id, get_pos.pos, dstpos);

      return true;
    }

    return false;
  }
  bool move_rel(Id obj_id, Vec2i delta) {
    auto obj = get_object(obj_id);
    if(obj) {
      GetPositionEvent get_pos;
      obj->fire_event(get_pos);

      auto dstpos = get_pos.pos + delta;

      for(auto & other_obj : all_objects) {
        if(other_obj && other_obj != obj) {
          GetPositionEvent event;
          other_obj->fire_event(event);

          if(event.pos == dstpos) {
            return false;
          }
        }
      }

      SetPositionEvent set_pos(dstpos);
      obj->fire_event(set_pos);

      view().on_agent_move(obj_id, get_pos.pos, dstpos);

      return true;
    }

    return false;
  }
}

