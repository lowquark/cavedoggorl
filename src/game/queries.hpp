#ifndef QUERIES_HPP
#define QUERIES_HPP

#include <game/nc.hpp>
#include <game/actions.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>

namespace game {
  using namespace nc;

  static constexpr TypeId ACTION_QUERY_ID = 1;
  static constexpr TypeId GLYPH_QUERY_ID = 2;

  struct ActionQuery : public Query {
    std::unique_ptr<Action> action;
    ActionQuery() : Query(ACTION_QUERY_ID) {}
  };
  struct GlyphQuery : public Query {
    unsigned int glyph_id = 0;
    Color color;
    GlyphQuery() : Query(GLYPH_QUERY_ID) {}
  };
};

#endif
