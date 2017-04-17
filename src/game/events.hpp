#ifndef GAME_EVENTS_HPP
#define GAME_EVENTS_HPP

#include <game/core.hpp>

namespace game {
  static const unsigned int EVENT_TYPE_ACTIVATE = 1;
  static const unsigned int EVENT_TYPE_IS_PASSABLE = 2;

  class ActivateEvent : public Event {
    public:
    ActivateEvent() : Event(EVENT_TYPE_ACTIVATE) {}
  };
  class IsPassableEvent : public Event {
    public:
    IsPassableEvent() : Event(EVENT_TYPE_IS_PASSABLE) {}
  };
}

#endif
