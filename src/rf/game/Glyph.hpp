#ifndef RF_GAME_GLYPH_HPP
#define RF_GAME_GLYPH_HPP

#include "Color.hpp"

namespace rf {
  namespace game {
    struct Glyph {
      unsigned int index = 0;
      Color foreground;
      Color background;

      Glyph() = default;
      Glyph(unsigned int index, const Color & fg, const Color & bg = Color())
        : index(index), foreground(fg), background(bg) {}
    };
  }
}

#endif
