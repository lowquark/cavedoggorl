#ifndef GAME_PROPERTIES_HPP
#define GAME_PROPERTIES_HPP

#include <game/core.hpp>
#include <game/events.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>

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
          fire_event(static_cast<const SetPositionEvent &>(event));
          break;
      }
    }
  };

  struct SpritePart : public BasePart {
    SpritePart() : BasePart(PART_TYPE_SPRITE) {}

    Color color = Color(1.0f, 0.0f, 1.0f);
  };

  struct AgentPart : public BasePart {
    AgentPart() : BasePart(PART_TYPE_AGENT) {}

    int team = -1;

    unsigned int turn_time = 0;
  };

  struct AIPart : public BasePart {
    AIPart() : BasePart(PART_TYPE_AI) {}

    // AI
    std::vector<Vec2i> path;
  };

  struct PlayerPart : public BasePart {
    PlayerPart() : BasePart(PART_TYPE_PLAYER) {}
  };
}

#endif
