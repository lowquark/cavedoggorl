#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include <cstdint>
#include <util/Map.hpp>

#include <iostream>
#include <map>

namespace game {
  struct TileState {
    uint8_t type_id;
  };
  struct AgentState {
    int16_t x;
    int16_t y;
    uint8_t is_evil;
  };

  struct LevelState {
    Map<TileState> tiles;
    std::vector<AgentState> agents;
    uint16_t current_agent_idx = 0;

    void clear() {
      tiles.clear();
      agents.clear();
      current_agent_idx = 0;
    }

    bool is_valid() {
      return agents.size() == 0 || current_agent_idx < agents.size();
    }

    static void write(std::ostream & os, const LevelState & state);
    static bool read(std::istream & is, LevelState & state);
  };

  struct ObjectPhysicsData {
    int16_t x;
    int16_t y;
    uint8_t flags;

    static constexpr uint8_t BIG_FLAG = 0x01;
  };
  struct PhysicsState {
    Map<TileState> tiles;
    // object id => object physics
    std::map<uint16_t, ObjectPhysicsData> physics_data;

    void clear() {
      tiles.clear();
      physics_data.clear();
    }
  };

  // Time / Turns / Ticks
  struct AgentTurnState {
    uint16_t object_id;
    uint16_t time;
  };
  struct TurnState {
    // a vector is used here, because order is important
    std::vector<AgentTurnState> agent_turns;
    uint16_t turn_idx = 0;
  };
}

#endif
