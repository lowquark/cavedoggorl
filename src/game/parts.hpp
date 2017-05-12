#ifndef PARTS_HPP
#define PARTS_HPP

#include <sstream>
#include <cassert>
#include <deque>

#include <game/core.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>

namespace game {
  struct GlyphPart : public PartHelper<GlyphPart> {
    Color color;

    GlyphPart(const std::string & data) {
    }
  };
  struct SpatialPart : public PartHelper<SpatialPart> {
    Vec2i pos;

    SpatialPart(const std::string & data) {
      assert(data.substr(0, 7) == "Spatial");
      std::stringstream ss(data);
      ss >> pos.x;
      ss >> pos.y;
    }

    std::string serialize() const override {
      std::stringstream ss;
      ss << "Spatial";
      ss << " " << pos.x;
      ss << " " << pos.y;
      return ss.str();
    }
  };
  struct TurnTakerPart : public PartHelper<TurnTakerPart> {
    unsigned int wait_time = 0;
    bool break_turns = false;

    TurnTakerPart(const std::string & data) {
      assert(data.substr(0, 9) == "TurnTaker");

      std::stringstream ss(data.substr(9));
      ss >> wait_time;
    }

    std::string serialize() const override {
      std::stringstream ss;
      ss << "TurnTaker";
      ss << " " << wait_time;
      return ss.str();
    }
  };
  struct AgentPart : public PartHelper<AgentPart> {
    int hp = 0;
    int max_hp = 0;

    AgentPart(const std::string & data) {
    }
  };
}

#endif
