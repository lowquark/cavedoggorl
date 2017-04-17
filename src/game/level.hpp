#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP

#include <game/core.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>
#include <util/Map.hpp>
#include <vector>

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
  extern int current_agent_idx;

  Id create_object(const Object::Builder & builder);
  void destroy_object(Id id);
  void clear_objects();

  bool any_agents_playable();
  Agent * find_agent(Vec2i pos);
  bool can_move(Vec2i desired_pos);
  void move_attack(Agent & agent, Vec2i dst);

  void load_level();
}

#endif
