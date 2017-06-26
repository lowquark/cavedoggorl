#ifndef PARTS_HPP
#define PARTS_HPP

#include <sstream>
#include <cassert>
#include <deque>

#include <game/nc.hpp>
#include <game/actions.hpp>
#include <game/Color.hpp>
#include <game/FOV.hpp>
#include <util/Vec2.hpp>

#include <game/queries.hpp>

namespace game {
  using namespace nc;

  static constexpr TypeId GLYPH_PART_ID = 2;
  static constexpr TypeId RANDOM_CONTROL_PART_ID = 3;

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

    void query(GlyphQuery & q) const {
      printf("%s\n", __PRETTY_FUNCTION__);

      q.glyph_id = type_id;
      q.color = color;
      q.abort();
    }
    void query(Query & q) const override {
      switch(q.type_id()) {
        case GLYPH_QUERY_ID: query(static_cast<GlyphQuery&>(q)); break;
        default: break;
      }
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
  struct RandomControlPart : public Part {
    RandomControlPart(const std::string & data) : Part(RANDOM_CONTROL_PART_ID) {}

    void query(ActionQuery & q) const {
      printf("%s\n", __PRETTY_FUNCTION__);

      Vec2i delta((rand() % 3) - 1, (rand() % 3) - 1);
      q.action = std::unique_ptr<MoveAction>(new MoveAction(delta));
      q.abort();
    }
    void query(Query & q) const override {
      switch(q.type_id()) {
        case ACTION_QUERY_ID: query(static_cast<ActionQuery&>(q)); break;
        default: break;
      }
    }
  };
}

#endif
