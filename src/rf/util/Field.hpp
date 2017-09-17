#ifndef RF_UTIL_FIELD_HPP
#define RF_UTIL_FIELD_HPP

#include "Vec2.hpp"
#include <vector>
#include <cassert>
#include <stdexcept>
#include <limits>

namespace rf {
  template <typename T>
  class Field {
    public:
    Field() noexcept {}

    void clear() noexcept {
      _rect = Rect2i();
      _data.clear();
      _exterior = T();
    }
    void define(Rect2i rect, const std::vector<T> & data, const T & exterior) noexcept {
      assert(rect.size.x <= std::numeric_limits<unsigned int>::max()/rect.size.y);
      assert(data.size() == rect.size.x * rect.size.y);

      _rect = rect;
      _data = data;
      _exterior = exterior;
    }

    void fill(const T & t) noexcept {
      std::fill(_data.begin(), _data.end(), t);
      _exterior = t;
    }

    bool valid(Vec2i pos) const noexcept {
      if(pos.x <  _rect.pos.x) { return false; }
      if(pos.y <  _rect.pos.y) { return false; }
      if(pos.x >= _rect.pos.x + _rect.size.x) { return false; }
      if(pos.y >= _rect.pos.y + _rect.size.y) { return false; }
      return true;
    }
    unsigned int index(Vec2i pos) const noexcept {
      return (pos.x - _rect.pos.x) + (pos.y - _rect.pos.y)*_rect.size.x;
    }

    const T & operator[](Vec2i pos) const noexcept {
      if(valid(pos)) {
        return _data[index(pos)];
      } else {
        return _exterior;
      }
    }
    T & operator[](Vec2i pos) noexcept {
      return _data[index(pos)];
    }

    const T & at(Vec2i pos) const noexcept {
      return operator[](pos);
    }
    T & at(Vec2i pos) {
      if(valid(pos)) {
        return _data[index(pos)];
      } else {
        throw std::out_of_range("Map::at(...)");
      }
    }

    private:
    Rect2i _rect;
    std::vector<T> _data;
    T _exterior;
  };
}

#endif
