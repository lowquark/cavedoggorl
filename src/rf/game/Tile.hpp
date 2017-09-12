#ifndef RF_GAME_TILE_HPP
#define RF_GAME_TILE_HPP

#include <vector>
#include <rf/game/Glyph.hpp>
#include <rf/game/Handlers.hpp>

namespace rf {
  namespace game {
    class TileHandlers {
      public:
      void add(const GlyphHandler & h) { glyph.push_back(&h); }

      private:
      std::vector<const GlyphHandler *> glyph;

      friend class Tile;
    };

    class TilePart {
      public:
      virtual ~TilePart() = default;
      virtual void init(TileHandlers & oh) {}
    };

    class BasicTileGlyph : public TilePart {
      class GlyphHandler : public game::GlyphHandler {
        Glyph g;
        Glyph operator()() const override {
          return g;
        }
        public:
        GlyphHandler(const Glyph & g) : g(g) {}
      };

      GlyphHandler glyph;

      void init(TileHandlers & oh) override {
        oh.add(glyph);
      }

      public:
      BasicTileGlyph(const Glyph & g) : glyph(g) {}
    };

    class Tile {
      std::vector<TilePart *> parts;

      TileHandlers handlers;

      public:
      Tile() = default;
      Tile(const Tile & other) = delete;
      Tile & operator=(const Tile & other) = delete;
      Tile(Tile && other) noexcept = default;
      Tile & operator=(Tile && other) noexcept = default;
      ~Tile() {
        for(auto & p : parts) {
          delete p;
        }
      }
      // Part must be allocated via `operator new`
      void add(TilePart * p) {
        parts.push_back(p);
        p->init(handlers);
      }

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
    };
  }
}

#endif
