#ifndef RF_UTIL_MAP_HPP
#define RF_UTIL_MAP_HPP

#include "Vec2.hpp"
#include <vector>
#include <stdexcept>

namespace rf {
  template <typename T>
  class Map {
    public:
    Map() noexcept {}
    Map(Vec2u size) {
      resize(size);
    }

    void clear() noexcept {
      _size = Vec2u(0, 0);
      _data.clear();
    }
    void resize(Vec2u size) {
      _size = size;
      _data.resize(_size.x * _size.y);
    }

    bool valid(Vec2u pos) const noexcept {
      if(pos.x >= _size.x) { return false; }
      if(pos.y >= _size.y) { return false; }
      return true;
    }
    unsigned int index(Vec2u pos) const noexcept {
      return pos.x + pos.y*_size.x;
    }

    T & get(Vec2u pos) {
      if(valid(pos)) {
        return _data[index(pos)];
      } else {
        throw std::out_of_range("Map::get(...)");
      }
    }
    const T & get(Vec2u pos) const {
      if(valid(pos)) {
        return _data[index(pos)];
      } else {
        throw std::out_of_range("Map::get(...)");
      }
    }
    void set(Vec2u pos, const T & t) {
      if(valid(pos)) {
        _data[index(pos)] = t;
      } else {
        throw std::out_of_range("Map::set(...)");
      }
    }

    Vec2u size() const noexcept { return _size; }
    const T * data() const noexcept { return _data.data(); }
    T * data() noexcept { return _data.data(); }

    private:
    std::vector<T> _data;
    Vec2u _size;
  };
}

#endif
