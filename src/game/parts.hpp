#ifndef PARTS_HPP
#define PARTS_HPP

#include <sstream>
#include <cassert>
#include <deque>

#include <game/view.hpp>
#include <game/core.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>

namespace game {
  struct GlyphPart : public PartHelper<GlyphPart> {
    Vec2i pos;
    Color color = Color(0xFF, 0x00, 0xFF);

    static Id last_id;
    Id id = 0;
    Id type_id = 0;

    GlyphPart(const std::string & data) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Glyph");

      int type_id, x, y, r, g, b;
      ss >> type_id >> x >> y >> r >> g >> b;

      pos = Vec2i(x, y);
      color = Color(r, g, b);

      id = last_id ++;
      this->type_id = std::abs(type_id);

      view().on_agent_load(id, type_id, pos, color);
    }

    ~GlyphPart() {
      view().on_agent_death(0);
    }

    void on_move(Vec2i from, Vec2i to) {
      view().on_agent_move(id, from, to);
    }

    void look_at_me() {
      view().look_at(id);
    }

    std::string serialize() const override {
      std::stringstream ss;
      ss << "Glyph " << type_id
         << " " << pos.x
         << " " << pos.y
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
    PlayerPart(const std::string & data) {
    }
  };
}

#endif
