#ifndef GAME_VIEW_HPP
#define GAME_VIEW_HPP

#include <string>

#include <util/Log.hpp>
#include <util/Vec2.hpp>
#include <game/core.hpp>
#include <game/Color.hpp>

namespace game {
  struct View {
    virtual void on_world_load(unsigned int tiles_x, unsigned int tiles_y) {}

    virtual void on_tile_load(Id type_id, const Vec2i & pos) {}
    virtual void on_agent_load(Id agent_id, Id type_id, const Vec2i & pos, const Color & color) {}

    virtual void on_agent_move(Id agent_id, const Vec2i & from, const Vec2i & to) {}
    virtual void on_agent_death(Id agent_id) {}

    virtual void on_message(const std::string & message) {}

    virtual void on_control(Id agent_id) {}
  };

  void set_view(View & view);
  View & view();

  extern Log log;
}

#endif
