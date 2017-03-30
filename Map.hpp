#ifndef MAP_HPP
#define MAP_HPP

#include "Vec2.hpp"

template <typename T>
class Map {
  std::vector<T> _data;
  unsigned int _w, _h;

  public:
  Map() : _w(0), _h(0) {}

  Map(unsigned int w, unsigned int h, const T & fill = T()) : _w(0), _h(0) {
    resize(w, h, fill);
  }

  void resize(unsigned int w, unsigned int h, const T & fill = T()) {
    this->_w = w;
    this->_h = h;
    _data.resize(_w * _h, fill);
  }

  bool valid(Vec2i pos) const {
    if(pos.x < 0) { return false; }
    if(pos.x >= _w) { return false; }
    if(pos.y < 0) { return false; }
    if(pos.y >= _h) { return false; }

    return true;
  }
  unsigned int index(Vec2i pos) const {
    return pos.x + pos.y*_w;
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

  unsigned int w() const { return _w; }
  unsigned int h() const { return _h; }
  const std::vector<T> & data() const { return _data; }
};

#endif
