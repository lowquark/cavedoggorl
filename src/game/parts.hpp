#ifndef PARTS_HPP
#define PARTS_HPP

#include <sstream>
#include <cassert>
#include <deque>

#include <game/core.hpp>
#include <game/Color.hpp>
#include <game/FOV.hpp>
#include <util/Vec2.hpp>

#include <game/queries.hpp>

namespace game {
  using namespace newcore;

  static constexpr TypeId PHYS_PART_ID = 1;
  static constexpr TypeId GLYPH_PART_ID = 2;

  struct PhysPart : public Part {
    Vec2i pos;

    PhysPart(const std::string & data) : Part(PHYS_PART_ID) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Phys");

      int x, y;
      ss >> x >> y;

      pos = Vec2i(x, y);
    }
    static std::string state(Vec2i pos) {
      std::stringstream ss;
      ss << "Phys " << pos.x << " " << pos.y;
      return ss.str();
    }
    std::string serialize() const {
      std::stringstream ss;
      ss << "Phys " << pos.x << " " << pos.y;
      return ss.str();
    }
  };

  struct GlyphPart : public Part {
    unsigned int type_id = 0;
    Color color = Color(0xFF, 0x00, 0xFF);

    static int clamp(int x, int min, int max) {
      if(x > max)
        return max;
      if(x < min)
        return min;
      return x;
    }
    GlyphPart(const std::string & data) : Part(GLYPH_PART_ID) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Glyph");

      int type_id, r, g, b;
      ss >> type_id >> r >> g >> b;

      color = Color(clamp(r, 0, 255), clamp(g, 0, 255), clamp(b, 0, 255));

      this->type_id = std::abs(type_id);
    }

    /*
    std::string serialize() const {
      std::stringstream ss;
      ss << "Glyph " << type_id
         << " " << color.r
         << " " << color.g
         << " " << color.b;
      return ss.str();
    }
    */
  };
}

#endif
