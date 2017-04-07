#ifndef GFX_COLOR_HPP
#define GFX_COLOR_HPP

namespace gfx {
  struct Color {
    float r, g, b, a;

    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

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
