#ifndef GAME_EVENTS_HPP
#define GAME_EVENTS_HPP

#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  static const unsigned int EVENT_TYPE_ACTIVATE = 1;
  static const unsigned int EVENT_TYPE_IS_PASSABLE = 2;

  static const unsigned int EVENT_TYPE_GET_POSITION = 3;
  static const unsigned int EVENT_TYPE_SET_POSITION = 4;

  class ActivateEvent : public BaseEvent {
    public:
    ActivateEvent() : BaseEvent(EVENT_TYPE_ACTIVATE) {}
  };
  class IsPassableEvent : public BaseEvent {
    public:
    IsPassableEvent() : BaseEvent(EVENT_TYPE_IS_PASSABLE) {}
  };

  struct GetPositionEvent : public BaseEvent {
    GetPositionEvent() : BaseEvent(EVENT_TYPE_GET_POSITION) {}

    Vec2i pos;
  };
  struct SetPositionEvent : public BaseEvent {
    SetPositionEvent(Vec2i pos) : BaseEvent(EVENT_TYPE_SET_POSITION), pos(pos) {}

    const Vec2i pos;
  };
}

#endif
