#ifndef GAME_VIEW_HPP
#define GAME_VIEW_HPP

#include <string>

#include <util/Vec2.hpp>
#include <game/core.hpp>
#include <game/Color.hpp>
#include <game/FOV.hpp>

namespace game {
  struct View {
    virtual void spawn_space(nc::Id sid, Vec2u size) {}
    virtual void despawn_space(nc::Id sid) {}
    virtual void set_tile(nc::Id sid, const Vec2i & pos, unsigned int type_id) {}
    virtual void clear_tile(nc::Id sid, const Vec2i & pos) {}

    virtual void set_glyph(nc::Id eid,
                           unsigned int type_id,
                           const Vec2i & pos,
                           const Color & color) {}
    virtual void clear_glyph(nc::Id eid) {}
    virtual void move_glyph(nc::Id eid,
                            const Vec2i & from,
                            const Vec2i & to) {}

    virtual void message(const std::string & message) {}

    virtual void follow(nc::Id eid) {}

    virtual void set_fov(unsigned int id, const FOV & fov) {}

    virtual void clear() {}
  };
}

#endif
