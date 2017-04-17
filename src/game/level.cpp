
#include "level.hpp"

#include <game/view.hpp>

#include <cstdio>

namespace game {
  Map<Tile> tiles(40, 40);

  // We are all africans
  std::vector<Agent> all_agents;
  std::vector<Object *> objects;

  int current_agent_idx = 0;

  Id create_object(const Object::Builder & builder) {
    for(std::size_t idx = 0 ; idx < objects.size() ; idx ++) {
      auto o = objects[idx];
      if(o == nullptr) {
        o = new Object(builder.build());
        return idx + 1;
      }
    }
    objects.push_back(new Object(builder.build()));
    return objects.size();
  }
  void destroy_object(Id id) {
    if(id == 0) {
      return;
    }
    if(id > objects.size()) {
      return;
    }

    delete objects[id - 1];
    objects[id - 1] = nullptr;
  }
  void clear_objects() {
    for(auto & o : objects) {
      delete o;
    }
    objects.clear();
  }

  bool any_agents_playable() {
    for(auto & agent : all_agents) {
      if(agent.player_controlled) {
        return true;
      }
    }
    return false;
  }

  Agent * find_agent(Vec2i pos) {
    for(auto & agent : all_agents) {
      if(agent.pos == pos) {
        return &agent;
      }
    }
    return nullptr;
  }
  bool can_move(Vec2i desired_pos) {
    if(find_agent(desired_pos)) {
      return false;
    }

    return tiles.get(desired_pos).passable;
  }

  // Generic move attack
  void move_attack(Agent & agent, Vec2i dst) {
    Agent * other = find_agent(dst);
    if(other && other->team != agent.team) {
      other->hp -= 10;

      if(other->is_dead()) {
        char message[100];
        snprintf(message, sizeof(message), "Ouch! %p Is solidly dead.", other);
        view().on_message(message);

        log.logf("Ouch! %p Is solidly dead.", other);

        view().on_agent_death(other->id);
      } else {
        char message[100];
        snprintf(message, sizeof(message), "Ouch! %p lost 10 hp. (now at %d)", other, other->hp);
        view().on_message(message);

        log.logf("Ouch! %p lost 10 hp. (now at %d)", other, other->hp);
      }
    } else if(can_move(dst)) {
      view().on_agent_move(agent.id, agent.pos, dst);
      agent.pos = dst;
    }

    agent.time = (rand() % 5) + 4;
  }
}

