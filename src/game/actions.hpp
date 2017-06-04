#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include <util/Vec2.hpp>
#include <game/core.hpp>

namespace game {
  class Engine;

  class Action {
    public:
    virtual ~Action() = default;
    virtual void perform(Engine & E, nc::Id eid) const = 0;
  };

  class MoveAction : public Action {
    Vec2i delta;
    public:
    MoveAction(Vec2i delta) : delta(delta) {}
    void perform(Engine & E, nc::Id eid) const override;
  };

  class WaitAction : public Action {
    public:
    void perform(Engine & E, nc::Id eid) const override;
  };
}

#endif
