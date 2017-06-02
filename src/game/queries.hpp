#ifndef QUERIES_HPP
#define QUERIES_HPP

#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  using namespace newcore;

  static constexpr TypeId TURN_QUERY_ID = 1;
  static constexpr TypeId GLYPH_QUERY_ID = 2;

  struct TurnQuery : public Query {
    TurnQuery() : Query(TURN_QUERY_ID) {}
  };
  struct GlyphQuery : public Query {
    unsigned int glyph_id = 0;
    GlyphQuery() : Query(GLYPH_QUERY_ID) {}
  };
};

#endif
