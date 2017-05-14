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

    virtual void look_at(Id agent_id) {}
    virtual void look_at(Vec2i pos) {}

    //virtual void add_playable(Id agent_id) {}
    //virtual void playable_turn(Id agent_id) {}
    //virtual void remove_playable(Id agent_id) {}
  };

  extern Log log;
}

#endif
