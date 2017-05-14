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
    Id type_id = 0;
    Color color = Color(0xFF, 0x00, 0xFF);

    static int clamp(int x, int min, int max) {
      if(x > max)
        return max;
      if(x < min)
        return min;
      return x;
    }
    GlyphPart(const std::string & data) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Glyph");

      int type_id, r, g, b;
      ss >> type_id >> r >> g >> b;

      color = Color(clamp(r, 0, 255), clamp(g, 0, 255), clamp(b, 0, 255));

      this->type_id = std::abs(type_id);
    }

    std::string serialize() const override {
      std::stringstream ss;
      ss << "Glyph " << type_id
         << " " << color.r
         << " " << color.g
         << " " << color.b;
      return ss.str();
    }
  };

  struct SpatialPart : public PartHelper<SpatialPart> {
    Vec2i pos;

    SpatialPart(const std::string & data) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Spatial");

      int x, y;
      ss >> x >> y;

      pos = Vec2i(x, y);
    }

    std::string serialize() const override {
      std::stringstream ss;
      ss << "Spatial " << pos.x
         << " " << pos.y;
      return ss.str();
    }
  };
  struct TurnTakerPart : public PartHelper<TurnTakerPart> {
    unsigned int wait_time = 0;
    bool break_turns = false;

    TurnTakerPart(const std::string & data) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "TurnTaker");

      int wait_time;
      ss >> wait_time;

      this->wait_time = wait_time;
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
  struct PlayerPart : public PartHelper<PlayerPart> {
    bool needs_input = true;
    unsigned int player_id;

    PlayerPart(const std::string & data) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Player");

      int player_id;
      ss >> player_id;

      this->player_id = player_id;
    }
  };
}

#endif
