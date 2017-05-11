
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


  unsigned int turn_idx = 0;

  Id run_until_player_turn() {
    if(all_objects.empty()) { return 0; }

    bool any_playable = false;
    for(auto & obj : all_objects) {
      if(obj) {
        if(obj->get_part(PART_TYPE_PLAYER)) {
          any_playable = true;
          break;
        }
      }
    }

    if(any_playable == false) { return 0; }

    while(true) {
      turn_idx ++;

      if(turn_idx == all_objects.size()) {
        turn_idx = 0;
      }

      auto & obj = all_objects[turn_idx];
      if(obj) {
        TickEvent tick_ev;
        obj->fire_event(tick_ev);

        IsTurnReadyEvent is_turn_ready_ev;
        obj->fire_event(is_turn_ready_ev);

        if(is_turn_ready_ev.is_ready) {
          // since this is a player, we will not send it a TAKE_TURN event
          if(obj->get_part(PART_TYPE_PLAYER)) {
            printf("Player turn!\n");
            return turn_idx + 1;
          } else {
            printf("AI turn!\n");
          }
        }
      }
    }
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

      OnWalkEvent on_walk_ev;
      obj->fire_event(on_walk_ev);

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

      OnWalkEvent on_walk_ev;
      obj->fire_event(on_walk_ev);

      view().on_agent_move(obj_id, get_pos.pos, dstpos);

      return true;
    }

    return false;
  }
}

