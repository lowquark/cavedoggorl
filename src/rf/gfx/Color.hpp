#ifndef RF_GFX_COLOR_HPP
#define RF_GFX_COLOR_HPP

namespace rf {
  namespace gfx {
    struct Color {
      float r, g, b, a;

      Color() noexcept : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
      Color(float r, float g, float b) noexcept : r(r), g(g), b(b), a(1.0f) {}
      Color(float r, float g, float b, float a) noexcept : r(r), g(g), b(b), a(a) {}

      Color & operator+=(const Color & other) noexcept {
        this->r += other.r;
        this->g += other.g;
        this->b += other.b;
        return *this;
      }
      Color operator+(const Color & other) const noexcept {
        return Color(*this) += other;
      }

      Color & operator-=(const Color & other) noexcept {
        this->r -= other.r;
        this->g -= other.g;
        this->b -= other.b;
        return *this;
      }
      Color operator-(const Color & other) const noexcept {
        return Color(*this) -= other;
      }
    };
  }
}

#endif
