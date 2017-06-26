#ifndef GAME_ACTIONS_HPP
#define GAME_ACTIONS_HPP

#include <util/Vec2.hpp>
#include <game/common.hpp>

namespace game {
  class MobSys;

  class Action {
    public:
    virtual ~Action() = default;
    virtual void perform(MobSys & sys, Id eid) const = 0;
  };
  class MoveAction : public Action {
    public:
    Vec2i delta;
    MoveAction(Vec2i delta) : delta(delta) {}
    void perform(MobSys & sys, Id eid) const override;
  };
  class WaitAction : public Action {
    public:
    void perform(MobSys & sys, Id eid) const override;
  };
  class StairAction : public Action {
    public:
    void perform(MobSys & sys, Id eid) const override;
  };
}

#endif
