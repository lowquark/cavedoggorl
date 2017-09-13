#ifndef RF_GAME_OBJECT_HPP
#define RF_GAME_OBJECT_HPP

#include <rf/util/Vec2.hpp>
#include <rf/game/Glyph.hpp>
#include <rf/game/Handlers.hpp>

namespace rf {
  namespace game {
    class ObjectHandlers {
      public:
      void add(const GlyphHandler & h) { glyph.push_back(&h); }
      void add(const DeadHandler & h) { dead.push_back(&h); }
      void add(const PassiveDamageHandler & h) { passive_damage.push_back(&h); }

      void add(MoveHandler & h) { move.push_back(&h); }
      void add(DamageHandler & h) { damage.push_back(&h); }

      private:
      std::vector<const GlyphHandler *> glyph;
      std::vector<const DeadHandler *> dead;
      std::vector<const PassiveDamageHandler *> passive_damage;

      std::vector<MoveHandler *> move;
      std::vector<DamageHandler *> damage;

      friend class Object;
    };

    // attaches handlers, and is serializable ; exists to be deleted when the
    // object is deleted
    class ObjectPart {
      public:
      virtual ~ObjectPart() = default;
      virtual void init(ObjectHandlers & oh) {}
    };

    class BasicObjectGlyph : public ObjectPart {
      class GlyphHandler : public game::GlyphHandler {
        Glyph g;
        Glyph operator()() const override {
          return g;
        }
        public:
        GlyphHandler(const Glyph & g) : g(g) {}
      };

      GlyphHandler glyph;

      void init(ObjectHandlers & oh) override {
        oh.add(glyph);
      }

      public:
      BasicObjectGlyph(const Glyph & g) : glyph(g) {}
    };

    // extremely composed, homogeneous, universal game objects
    class Object {
      public:
      Object() = default;
      Object(const Object & other) = delete;
      Object & operator=(const Object & other) = delete;
      Object(Object && other) noexcept = default;
      Object & operator=(Object && other) noexcept = default;
      ~Object() {
        for(auto & p : parts) {
          delete p;
        }
      }
      // `p` must be allocated via `operator new`
      void add(ObjectPart * p) {
        parts.push_back(p);
        p->init(handlers);
      }

      Vec2i pos() const {
        return _pos;
      }
      void set_pos(Vec2i pos) {
        _pos = pos;
      }

      bool has_turn() const {
        return _has_turn;
      }
      void set_has_turn(bool b) {
        _has_turn = b;
      }
      int turn_energy() const {
        return _turn_energy;
      }
      void add_turn_energy(int amt) {
        _turn_energy += amt;
      }
      void use_turn_energy(int amt) {
        _turn_energy -= amt;
      }

      bool playable() const { return _playable; }
      void set_playable(bool b) { _playable = b; }

      std::vector<Glyph> glyphs() const {
        std::vector<Glyph> ret;
        for(auto & h : handlers.glyph) {
          ret.push_back((*h)());
        }
        return ret;
      };
      Glyph glyph() const {
        if(!handlers.glyph.empty()) {
          auto * h = handlers.glyph.back();
          return (*h)();
        } else {
          return Glyph();
        }
      };

      bool dead() const {
        for(auto & h : handlers.dead) {
          if((*h)()) { return true; }
        }
        return false;
      }
      int passive_damage() const {
        int sum = 0;
        for(auto & h : handlers.passive_damage) {
          sum += (*h)();
        }
        return sum;
      }

      // events
      void move(Vec2i pos) {
        for(auto & h : handlers.move) {
          (*h)(pos);
        }
      }
      void damage(int d) {
        for(auto & h : handlers.damage) {
          (*h)(d);
        }
      }

      private:
      std::vector<ObjectPart *> parts;

      ObjectHandlers handlers;

      Vec2i _pos;
      bool _has_turn = false;
      int _turn_energy = 0;

      bool _playable = false;
    };
  }
}

#endif
