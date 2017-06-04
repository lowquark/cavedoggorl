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
  static constexpr newcore::TypeId PHYS_PART_ID = 1;
  static constexpr newcore::TypeId GLYPH_PART_ID = 2;
  static constexpr newcore::TypeId TURN_PART_ID = 3;

  struct PhysPart : public newcore::Part {
    Vec2i pos;

    PhysPart(const std::string & data) : newcore::Part(PHYS_PART_ID) {
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
  struct TurnPart : public newcore::Part {
    unsigned int energy = 0;

    TurnPart(const std::string & data) : newcore::Part(TURN_PART_ID) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Turn");

      int energy;
      ss >> energy;

      this->energy = (unsigned int)energy;
    }

    static std::string state(unsigned int energy) {
      std::stringstream ss;
      ss << "Turn " << energy;
      return ss.str();
    }

    void add_energy(unsigned int amt) {
      energy += amt;
    }

    bool take() {
      if(energy >= 100) {
        energy -= 100;
        return true;
      }
      return false;
    }
  };

  struct GlyphPart : public PartHelper<GlyphPart>, public newcore::Part {
    unsigned int type_id = 0;
    Color color = Color(0xFF, 0x00, 0xFF);

    static int clamp(int x, int min, int max) {
      if(x > max)
        return max;
      if(x < min)
        return min;
      return x;
    }
    GlyphPart(const std::string & data) : newcore::Part(GLYPH_PART_ID) {
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

  struct SpatialPart : public PartHelper<SpatialPart>, public newcore::Part {
    Vec2i pos;

    SpatialPart(const std::string & data) : newcore::Part(102) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "Spatial");

      int x, y;
      ss >> x >> y;

      pos = Vec2i(x, y);
    }

    static std::string state(Vec2i pos) {
      std::stringstream ss;
      ss << "Spatial " << pos.x
         << " " << pos.y;
      return ss.str();
    }
    std::string serialize() const override {
      std::stringstream ss;
      ss << "Spatial " << pos.x
         << " " << pos.y;
      return ss.str();
    }
  };
  struct TurnTakerPart : public PartHelper<TurnTakerPart>, public newcore::Part {
    unsigned int wait_time = 0;
    bool break_turns = false;

    TurnTakerPart(const std::string & data) : newcore::Part(103) {
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
  struct AgentPart : public PartHelper<AgentPart>, public newcore::Part {
    int hp = 0;
    int max_hp = 0;
    BresenhamFOV fov;

    AgentPart(const std::string & data) : newcore::Part(104) {
    }
  };
  struct PlayerControlPart : public PartHelper<PlayerControlPart>, public newcore::Part {
    unsigned int player_id;

    PlayerControlPart(const std::string & data) : newcore::Part(105) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "PlayerControl");

      int player_id;
      ss >> player_id;

      this->player_id = player_id;
    }
  };
  struct AIControlPart : public PartHelper<AIControlPart>, public newcore::Part {
    std::vector<Vec2i> path;
    ObjectHandle kill_obj;

    AIControlPart(const std::string & data) : newcore::Part(106) {
      std::stringstream ss(data);

      std::string name;
      ss >> name;
      assert(name == "AIControl");

      unsigned int kill_obj;
      ss >> kill_obj;

      this->kill_obj = ObjectHandle(kill_obj);
    }

    static std::string state(ObjectHandle kill_obj) {
      std::stringstream ss;
      ss << "AIControl " << kill_obj.id();
      return ss.str();
    }
  };
}

#endif
