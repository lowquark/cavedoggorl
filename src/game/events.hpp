#ifndef GAME_EVENTS_HPP
#define GAME_EVENTS_HPP

#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  static const unsigned int EVENT_TYPE_QUERY_BREAK = 1;

  static const unsigned int EVENT_TYPE_GET_POSITION = 3;
  static const unsigned int EVENT_TYPE_SET_POSITION = 4;

  static const unsigned int EVENT_TYPE_TICK = 5;
  static const unsigned int EVENT_TYPE_IS_TURN_READY = 6;

  static const unsigned int EVENT_TYPE_ON_WALK = 10;

  static const unsigned int EVENT_TYPE_ENEMY_SEEN = 20;
  static const unsigned int EVENT_TYPE_ENEMY_LOST = 21;

  struct QueryBreakEvent : public BaseEvent {
    QueryBreakEvent() : BaseEvent(EVENT_TYPE_QUERY_BREAK) {}

    bool should_break = false;
  };

  struct GetPositionEvent : public BaseEvent {
    GetPositionEvent() : BaseEvent(EVENT_TYPE_GET_POSITION) {}

    Vec2i pos;
  };
  struct SetPositionEvent : public BaseEvent {
    SetPositionEvent(Vec2i pos) : BaseEvent(EVENT_TYPE_SET_POSITION), pos(pos) {}

    const Vec2i pos;
  };

  struct TickEvent : public BaseEvent {
    TickEvent() : BaseEvent(EVENT_TYPE_TICK) {}
  };

  struct IsTurnReadyEvent : public BaseEvent {
    IsTurnReadyEvent() : BaseEvent(EVENT_TYPE_IS_TURN_READY) {}

    bool is_ready = false;
  };

  struct OnWalkEvent : public BaseEvent {
    OnWalkEvent() : BaseEvent(EVENT_TYPE_ON_WALK) {}
  };

  struct EnemySeenEvent : public BaseEvent {
    EnemySeenEvent(Id obj_id) : BaseEvent(EVENT_TYPE_ENEMY_SEEN), obj_id(obj_id) {}

    Id obj_id = 0;
  };
  struct EnemyLostEvent : public BaseEvent {
    EnemyLostEvent(Id obj_id) : BaseEvent(EVENT_TYPE_ENEMY_LOST), obj_id(obj_id) {}

    Id obj_id = 0;
  };
}

#endif
