#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <string>
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

    virtual void on_message(const std::string & message) {}

    virtual void on_control(Id agent_id) {}
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

    bool player_controlled = false;

    // Rendering
    Color color = Color(1.0f, 0.0f, 1.0f);

    // AI
    std::vector<Vec2i> path;
  };

  // Moves the player controlled agent
  void move_attack(Vec2i delta);
  void step_game();

  bool is_over();

  void set_view(View & view);
  View & view();

  void load_world();


  namespace debug {
    const Agent * get_agent(Id id);
  }
}

#endif
