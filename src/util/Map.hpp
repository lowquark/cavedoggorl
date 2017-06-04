#ifndef MAP_HPP
#define MAP_HPP

#include "Vec2.hpp"
#include <vector>

template <typename T>
class Map {
  std::vector<T> _data;
  Vec2u _size;

  public:
  Map() = default;
  Map(Vec2u size, const T & fill = T()) {
    resize(size, fill);
  }

  void clear() {
    _size = Vec2u(0, 0);
    _data.clear();
  }
  void resize(Vec2u size, const T & fill = T()) {
    _size = size;
    _data.resize(_size.x * _size.y, fill);
  }

  bool valid(Vec2i pos) const {
    if(pos.x < 0) { return false; }
    if(pos.x >= _size.x) { return false; }
    if(pos.y < 0) { return false; }
    if(pos.y >= _size.y) { return false; }

    return true;
  }
  unsigned int index(Vec2i pos) const {
    return pos.x + pos.y*_size.x;
  }

  const T * get_ptr(Vec2i pos) const {
    if(valid(pos)) {
      return &_data[index(pos)];
    }
    return nullptr;
  }
  T * get_ptr(Vec2i pos) {
    if(valid(pos)) {
      return &_data[index(pos)];
    }
    return nullptr;
  }

  const T & get(Vec2i pos, const T & _default = T()) const {
    if(valid(pos)) {
      return _data[index(pos)];
    }
    return _default;
  }
  void set(Vec2i pos, const T & t) {
    if(valid(pos)) {
      _data[index(pos)] = t;
    }
  }

  Vec2u size() const { return _size; }
  const std::vector<T> & data() const { return _data; }
};

#endif
