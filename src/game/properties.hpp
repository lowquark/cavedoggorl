#ifndef GAME_PROPERTIES_HPP
#define GAME_PROPERTIES_HPP

#include <game/core.hpp>
#include <game/events.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>

#include <set>

namespace game {
  static const unsigned int PART_TYPE_PHYSICS = 1;
  static const unsigned int PART_TYPE_AGENT = 2;
  static const unsigned int PART_TYPE_SPRITE = 3;
  static const unsigned int PART_TYPE_AI = 4;
  static const unsigned int PART_TYPE_PLAYER = 5;

  struct PhysicsPart : public BasePart {
    PhysicsPart() : BasePart(PART_TYPE_PHYSICS) {}

    Vec2i pos;
    bool big;

    int hp_max = 100;
    int hp = 100;

    void set_position(Vec2i pos) {
      this->pos = pos;
    }
    Vec2i get_position() const {
      return this->pos;
    }

    void fire_event(GetPositionEvent & event) {
      event.pos = get_position();
    }
    void fire_event(const SetPositionEvent & event) {
      set_position(event.pos);
    }

    void fire_event(BaseEvent & event) override {
      switch(event.type_id()) {
        default:
          break;
        case EVENT_TYPE_GET_POSITION:
          fire_event(static_cast<GetPositionEvent &>(event));
          break;
        case EVENT_TYPE_SET_POSITION:
          fire_event(static_cast<SetPositionEvent &>(event));
          break;
      }
    }
  };

  struct AgentPart : public BasePart {
    AgentPart() : BasePart(PART_TYPE_AGENT) {}

    int team = -1;

    unsigned int turn_time = 0;


    void fire_event(const OnWalkEvent & event) {
      turn_time += 10;
    }
    void fire_event(const TickEvent & event) {
      if(turn_time > 0) {
        turn_time --;
      }
    }
    void fire_event(IsTurnReadyEvent & event) {
      event.is_ready = turn_time == 0;
    }

    void fire_event(BaseEvent & event) override {
      switch(event.type_id()) {
        default:
          break;
        case EVENT_TYPE_ON_WALK:
          fire_event(static_cast<OnWalkEvent &>(event));
          break;
        case EVENT_TYPE_TICK:
          fire_event(static_cast<TickEvent &>(event));
          break;
        case EVENT_TYPE_IS_TURN_READY:
          fire_event(static_cast<IsTurnReadyEvent &>(event));
          break;
      }
    }
  };

  struct SpritePart : public BasePart {
    SpritePart() : BasePart(PART_TYPE_SPRITE) {}

    Color color = Color(1.0f, 0.0f, 1.0f);
  };

  struct AIPart : public BasePart {
    AIPart() : BasePart(PART_TYPE_AI) {}

    std::vector<Vec2i> path;

    std::set<Id> enemy_ids;

    void fire_event(EnemySeenEvent & event) {
      printf("Sees the enemy!\n");
      enemy_ids.insert(event.obj_id);
    }
    void fire_event(EnemyLostEvent & event) {
      printf("Lost the enemy!\n");
      enemy_ids.erase(event.obj_id);
    }

    void fire_event(BaseEvent & event) override {
      switch(event.type_id()) {
        default:
          break;
        case EVENT_TYPE_ENEMY_SEEN:
          fire_event(static_cast<EnemySeenEvent &>(event));
          break;
        case EVENT_TYPE_ENEMY_LOST:
          fire_event(static_cast<EnemyLostEvent &>(event));
          break;
      }
    }
  };

  struct PlayerPart : public BasePart {
    PlayerPart() : BasePart(PART_TYPE_PLAYER) {}

    void fire_event(QueryBreakEvent & event) {
      IsTurnReadyEvent is_turn_ready_ev;
      fire_event(is_turn_ready_ev);
      event.should_break = is_turn_ready_ev.is_ready;
    }

    void fire_event(BaseEvent & event) override {
      switch(event.type_id()) {
        default:
          break;
        case EVENT_TYPE_QUERY_BREAK:
          fire_event(static_cast<QueryBreakEvent &>(event));
          break;
      }
    }
  };
}

#endif
