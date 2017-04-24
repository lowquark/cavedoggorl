#ifndef GAME_LEVEL_HPP
#define GAME_LEVEL_HPP

#include <game/core.hpp>
#include <game/state.hpp>
#include <game/Color.hpp>
#include <util/Vec2.hpp>
#include <util/Map.hpp>

#include <vector>
#include <map>

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


  struct ObjectPhysics : public Property {
    Vec2i pos;
    bool big = false;
  };

  struct PhysicsSystem {
    std::map<Id, ObjectPhysics> object_physics;

    void add(const Object & object) {
      if(find(dstpos) == 0) {
        position_map[id] = dstpos;
      }
    }
    void remove(Id id) {
      position_map.erase(id);
    }

    Id find(Vec2i pos) {
      for(auto & kvpair : position_map) {
        if(kvpair.second == pos) {
          return kvpair.first;
        }
      }
      return 0;
    }
    void move_to(Id id, Vec2i dstpos) {
      if(find(dstpos) == 0) {
        position_map[id] = dstpos;
      }
    }
  };


  struct Level {
    Map<Tile> tiles;
    std::vector<Agent *> all_agents;
    std::vector<Object *> objects;
    unsigned int current_agent_idx;

    public:
    // state
    void clear();
    void load(const LevelState & state);
    LevelState state();

    // queries
    Id find(Vec2i pos);
    bool can_move(Id agent_id, Vec2i desired_pos);

    Agent * get_agent(Id agent_id);

    // actions
    Id create(const Object::Builder & builder, Vec2i pos);
    bool move(Id agent_id, Vec2i dst);
    void destroy(Id agent_id);

    static Id move(Id src_id, const Level & src, const Level & dst);
  };
}

#endif
