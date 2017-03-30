#ifndef COLOR_HPP
#define COLOR_HPP

struct Color {
  float r, g, b;

  Color() : r(0.0f), g(0.0f), b(0.0f) {}
  Color(float r, float g, float b) : r(r), g(g), b(b) {}

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

#endif
