#ifndef VEC2_HPP
#define VEC2_HPP

template <typename T>
struct Vec2 {
  T x, y;

  Vec2() : x(0), y(0) {}
  Vec2(T x, T y) : x(x), y(y) {}

  template <typename U>
  Vec2(const Vec2<U> & other) : x(other.x), y(other.y) {}
  template <typename U>
  Vec2 & operator=(const Vec2<U> & other) {
    x = other.x;
    y = other.y;
    return *this;
  }

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

  Vec2 & operator*=(const T & scalar) {
    this->x *= scalar;
    this->y *= scalar;
    return *this;
  }
  Vec2 operator*(const T & scalar) const {
    return Vec2(*this) *= scalar;
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
typedef Vec2<float> Vec2f;
typedef Vec2<double> Vec2d;

template <typename T>
struct Rect2 {
  Vec2<T> pos;
  Vec2<T> size;

  Rect2() = default;
  Rect2(T x, T y, T w, T h)
    : pos(x, y)
    , size(w, h) {}
  Rect2(const Vec2<T> & pos, const Vec2<T> & size)
    : pos(pos)
    , size(size) {}
};

typedef Rect2<int> Rect2i;
typedef Rect2<unsigned int> Rect2u;
typedef Rect2<float> Rect2f;
typedef Rect2<double> Rect2d;

#endif
