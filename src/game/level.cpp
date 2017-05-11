
#include "level.hpp"

#include <game/view.hpp>
#include <util/serial.hpp>

namespace game {
  unsigned int turn_idx = 0;

  Id run_until_player_turn() {
    return turn_idx;
    /*
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
    */
  }

  bool move_to(Id obj_id, Vec2i dstpos) {
    return false;
  }
  bool move_rel(Id obj_id, Vec2i delta) {
    return false;
  }
}

