#ifndef RF_GAME_HANDLERS_HPP
#define RF_GAME_HANDLERS_HPP

#include <rf/util/Vec2.hpp>
#include <rf/game/Glyph.hpp>

namespace rf {
  namespace game {
    // Queries
    class GlyphHandler {
      public:
      virtual ~GlyphHandler() = default;
      virtual Glyph operator()() const { return Glyph(); }
    };

    class DeadHandler {
      public:
      virtual ~DeadHandler() = default;
      virtual bool operator()() const { return false; }
    };
    class PassiveDamageHandler {
      public:
      virtual ~PassiveDamageHandler() = default;
      virtual int operator()() const { return 0; }
    };

    // Events
    class MoveHandler {
      public:
      virtual ~MoveHandler() = default;
      virtual void operator()(Vec2i target) {}
    };
    class DamageHandler {
      public:
      virtual ~DamageHandler() = default;
      virtual void operator()(int d) {}
    };
  }
}

#endif
