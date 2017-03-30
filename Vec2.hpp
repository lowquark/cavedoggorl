#ifndef VEC2_HPP
#define VEC2_HPP

template <typename T>
struct Vec2 {
  T x, y;

  Vec2() : x(0), y(0) {}
  Vec2(T x, T y) : x(x), y(y) {}

  Vec2 & operator+=(const Vec2 & other) {
    this->x += other.x;
    this->y += other.y;
    return *this;
  }
  Vec2 operator+(const Vec2 & other) const {
    return Vec2(*this) += other;
  }

  Vec2 & operator-=(const Vec2 & other) {
    this->x -= other.x;
    this->y -= other.y;
    return *this;
  }
  Vec2 operator-(const Vec2 & other) const {
    return Vec2(*this) -= other;
  }

  bool operator==(const Vec2 & other) const {
    return this->x == other.x &&
           this->y == other.y;
  }
  bool operator!=(const Vec2 & other) const {
    return !(*this == other);
  }
};

typedef Vec2<int> Vec2i;
typedef Vec2<unsigned int> Vec2u;

#endif
