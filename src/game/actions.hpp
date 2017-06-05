#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include <util/Vec2.hpp>
#include <game/World.hpp>

namespace game {
  class MobSys;

  class Action {
    public:
    virtual ~Action() = default;
    virtual void perform(MobSys & sys, WorldId eid) const = 0;
  };
  class MoveAction : public Action {
    Vec2i delta;
    public:
    MoveAction(Vec2i delta) : delta(delta) {}
    void perform(MobSys & sys, WorldId eid) const override;
  };
  class WaitAction : public Action {
    public:
    void perform(MobSys & sys, WorldId eid) const override;
  };
}

#endif
