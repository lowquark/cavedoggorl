#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include "Color.hpp"
#include "Vec2.hpp"

namespace game {
  typedef unsigned long Id;

  struct View {
    virtual void on_world_load(unsigned int tiles_x, unsigned int tiles_y) {}

    virtual void on_tile_load(Id type_id, const Vec2i & pos) {}
    virtual void on_agent_load(Id agent_id, Id type_id, const Vec2i & pos, const Color & color) {}

    virtual void on_agent_move(Id agent_id, const Vec2i & from, const Vec2i & to) {}
    virtual void on_agent_death(Id agent_id) {}
  };


  struct Tile {
    enum Type { EMPTY, WALL, FLOOR };
    Type type = EMPTY;
    bool passable = false;
  };

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

    // Rendering
    Color color;

    // AI
    std::vector<Vec2i> path;

    Agent() : color(1.0f, 0.0f, 1.0f) {}
  };

  extern Agent player_agent;
  void move_attack(Agent & agent, Vec2i dst);
  void view(View * view);

  void load_world();

  void step_game();

  namespace debug {
    const Agent * get_agent(Id id);
  }
}

#endif
