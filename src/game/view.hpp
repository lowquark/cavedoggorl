#ifndef GAME_VIEW_HPP
#define GAME_VIEW_HPP

#include <string>

#include <util/Log.hpp>
#include <util/Map.hpp>
#include <util/Vec2.hpp>
#include <game/core.hpp>
#include <game/Color.hpp>

namespace game {
  struct View {
    virtual void set_world_size(unsigned int tiles_x, unsigned int tiles_y) {}
    virtual void set_tile(const Vec2i & pos, unsigned int type_id) {}

    virtual void set_glyph(ObjectHandle obj,
                           unsigned int type_id,
                           const Vec2i & pos,
                           const Color & color) {}
    virtual void move_glyph(ObjectHandle obj,
                            const Vec2i & from,
                            const Vec2i & to) {}
    virtual void remove_glyph(ObjectHandle obj) {}

    virtual void message(const std::string & message) {}

    virtual void follow(ObjectHandle obj) {}

    virtual void clear() {}
  };

  extern Log log;
}

#endif
