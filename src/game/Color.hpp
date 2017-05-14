#ifndef GAME_COLOR_HPP
#define GAME_COLOR_HPP

#include <cstdint>

namespace game {
  struct Color {
    uint8_t r, g, b;

    Color() : r(0), g(0), b(0) {}
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    Color & operator+=(const Color & other) {
      this->r += other.r;
      this->g += other.g;
      this->b += other.b;
      return *this;
    }
    Color operator+(const Color & other) const {
      return Color(*this) += other;
    }

    Color & operator-=(const Color & other) {
      this->r -= other.r;
      this->g -= other.g;
      this->b -= other.b;
      return *this;
    }
    Color operator-(const Color & other) const {
      return Color(*this) -= other;
    }
  };
}

#endif
