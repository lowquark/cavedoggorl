#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP

#include <game/core.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>
#include <util/Map.hpp>

#include <vector>
#include <iostream>

namespace game {
  struct Agent {
    Id id = 0;
    // Game mechanics
    Vec2i pos;

    unsigned int speed;
    int team = -1;

    int hp_max = 100;
    int hp = 100;
    bool is_dead() const { return hp <= 0; }

    int time = 0;

    bool player_controlled = false;

    // Rendering
    Color color = Color(1.0f, 0.0f, 1.0f);

    // AI
    std::vector<Vec2i> path;
  };

  struct Tile {
    enum Type { EMPTY, WALL, FLOOR };
    Type type = EMPTY;

    bool passable = false;

    Id object = 0;
  };

  extern Map<Tile> tiles;
  extern std::vector<Agent> all_agents;
  extern std::vector<Object *> objects;
  extern unsigned int current_agent_idx;


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

  Id create_object(const Object::Builder & builder);
  void destroy_object(Id id);
  void clear_objects();

  bool any_agents_playable();
  Agent * find_agent(Vec2i pos);
  bool can_move(Vec2i desired_pos);
  void move_attack(Agent & agent, Vec2i dst);

  void step_level();

  void load_level(const LevelState & state);
  LevelState level_state();

  namespace debug {
    const Agent * get_agent(Id id);
  }
}

#endif
