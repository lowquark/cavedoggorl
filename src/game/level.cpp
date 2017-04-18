
#include "level.hpp"

#include <game/view.hpp>
#include <game/events.hpp>
#include <game/properties.hpp>
#include <game/AStar.hpp>
#include <iostream>

#include <cstdio>

namespace serial {
  void write(std::ostream & os, uint8_t v) {
    os.write((const char *)&v, 1);
  }
  bool read(std::istream & is, uint8_t & v) {
    is.read((char *)&v, 1);
    return !is.fail();
  }
  void write(std::ostream & os, int8_t v) {
    write(os, (uint8_t)v);
  }
  bool read(std::istream & is, int8_t & v) {
    return read(is, (uint8_t &)v);
  }

  void write(std::ostream & os, uint16_t v) {
    uint8_t data[2];
    data[0] = (v >> 8) & 0xFF;
    data[1] = (v     ) & 0xFF;
    os.write((const char *)data, 2);
  }
  bool read(std::istream & is, uint16_t & v) {
    uint8_t data[2];
    is.read((char *)&data, 2);
    if(is.fail()) { return false; }
    v = (data[0] << 8) | data[1];
    return true;
  }
  void write(std::ostream & os, int16_t v) {
    write(os, (uint16_t)v);
  }
  bool read(std::istream & is, int16_t & v) {
    return read(is, (uint16_t &)v);
  }
}

namespace game {
  void LevelState::write(std::ostream & os, const LevelState & state) {
    uint16_t tiles_w = std::min(state.tiles.w(), (unsigned int)UINT16_MAX);
    uint16_t tiles_h = std::min(state.tiles.h(), (unsigned int)UINT16_MAX);

    serial::write(os, tiles_w);
    serial::write(os, tiles_h);

    for(unsigned int j = 0 ; j < tiles_h ; j ++) {
      for(unsigned int i = 0 ; i < tiles_w ; i ++) {
        auto & tile = state.tiles.get(Vec2i(i, j));
        serial::write(os, tile.type_id);
      }
    }

    uint16_t agents_size = std::min(state.agents.size(), (std::size_t)UINT16_MAX);

    serial::write(os, agents_size);

    for(auto & agent_state : state.agents) {
      serial::write(os, agent_state.x);
      serial::write(os, agent_state.y);
      serial::write(os, agent_state.is_evil);
    }

    serial::write(os, state.current_agent_idx);
  }
  bool LevelState::read(std::istream & is, LevelState & dst_state) {
    LevelState tmp_state;

    uint16_t tiles_w, tiles_h;
    if(!serial::read(is, tiles_w)) { return false; }
    if(!serial::read(is, tiles_h)) { return false; }

    tmp_state.tiles.resize(tiles_w, tiles_h);

    for(unsigned int j = 0 ; j < tiles_h ; j ++) {
      for(unsigned int i = 0 ; i < tiles_w ; i ++) {
        TileState tile_state;

        if(!serial::read(is, tile_state.type_id)) { return false; }

        tmp_state.tiles.set(Vec2i(i, j), tile_state);
      }
    }

    uint16_t agents_size;
    if(!serial::read(is, agents_size)) { return false; }

    for(unsigned int i = 0 ; i < agents_size ; i ++) {
      AgentState agent_state;

      if(!serial::read(is, agent_state.x)) { return false; }
      if(!serial::read(is, agent_state.y)) { return false; }
      if(!serial::read(is, agent_state.is_evil)) { return false; }

      printf("%d, %d, %u\n", agent_state.x, agent_state.y, agent_state.is_evil);

      tmp_state.agents.push_back(agent_state);
    }

    if(!serial::read(is, tmp_state.current_agent_idx)) { return false; }

    dst_state = std::move(tmp_state);

    return true;
  }

  Map<Tile> tiles(40, 40);

  // We are all africans
  std::vector<Agent> all_agents;
  std::vector<Object *> objects;

  unsigned int current_agent_idx = 0;

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

  void ai_turn(Agent & agent) {
    Map<unsigned int> tile_costs(tiles.w(), tiles.h());
    for(int y = 0 ; y < tile_costs.h() ; y ++) {
      for(int x = 0 ; x < tile_costs.w() ; x ++) {
        unsigned int cost = tiles.get(Vec2i(x, y)).passable ? 1 : 1000;
        for(auto & agent : all_agents) {
          if(agent.pos == Vec2i(x, y)) {
            cost += 4;
          }
        }

        tile_costs.set(Vec2i(x, y), cost);
      }
    }

    // TODO: Actual target-based following
    auto & player_agent = all_agents[0];
    DoAStar4(agent.path, tile_costs, player_agent.pos, agent.pos);

    if(agent.path.size() > 1) {
      Vec2i next_pos = agent.path[1];
      move_attack(agent, next_pos);
    } else {
      agent.time = 1;
    }
  }

  void step_level() {
    if(!any_agents_playable()) {
      log.log<Log::ERROR>("Cannot step the game without any player controlled agents.");
      return;
    }

    log.logf<Log::DEBUG1>("player turn complete!");

    while(true) {
      auto & agent = all_agents[current_agent_idx];

      agent.time --;
      if(agent.time <= 0) {
        if(agent.player_controlled) {
          log.logf<Log::DEBUG1>("time for player turn!");
          view().on_control(agent.id);
          return;
        } else {
          if(!agent.is_dead()) {
            log.logf<Log::DEBUG1>("time for ai turn! %p", &agent);
            ai_turn(agent);
            log.logf<Log::DEBUG1>("ai turn complete! %p", &agent);
          }
        }
      } 

      current_agent_idx ++;
      if(current_agent_idx >= all_agents.size()) {
        current_agent_idx = 0;

        for(auto it = all_agents.begin() ;
            it != all_agents.end() ; ) {
          if(it->is_dead()) {
            it = all_agents.erase(it);
          } else {
            it ++;
          }
        }

        if(!any_agents_playable()) {
          view().on_message("No more playable agents! Game over!");
          return;
        }
      }
    }
  }

  void clear_level() {
    tiles.clear();
    all_agents.clear();
    current_agent_idx = 0;
  }
  void load_level(const LevelState & state) {
    Object o = Object::Builder(property_factory)
                .add_property(PROPERTY_TYPE_OBSTRUCTION)
                .build();

    clear_level();

    unsigned int tiles_w = state.tiles.w();
    unsigned int tiles_h = state.tiles.h();
    tiles.resize(tiles_w, tiles_h);
    view().on_world_load(tiles_w, tiles_h);

    Tile wall_tile;
    wall_tile.type = Tile::WALL;
    wall_tile.passable = false;
    Tile floor_tile;
    floor_tile.type = Tile::FLOOR;
    floor_tile.passable = true;

    for(unsigned int j = 0 ; j < tiles_h ; j ++) {
      for(unsigned int i = 0 ; i < tiles_w ; i ++) {
        Vec2i pos(i, j);
        auto type_id = state.tiles.get(pos).type_id;
        if(type_id == 0) {
          tiles.set(pos, floor_tile);
          view().on_tile_load(floor_tile.type, pos);
        } else if(type_id == 1) {
          tiles.set(pos, wall_tile);
          view().on_tile_load(wall_tile.type, pos);
        }
      }
    }

    for(auto & agent : state.agents) {
      if(agent.is_evil) {
        all_agents.emplace_back();

        auto & impostor = all_agents.back();
        impostor.id = all_agents.size();
        impostor.pos = Vec2i(agent.x, agent.y);
        impostor.color = Color(50, 50, 50) + Color((float) 205 * rand() / RAND_MAX,
                                                   (float) 205 * rand() / RAND_MAX,
                                                   (float) 205 * rand() / RAND_MAX);
        impostor.team = 1;

        view().on_agent_load(impostor.id, 1, impostor.pos, impostor.color);
      } else {
        all_agents.emplace_back();

        auto & player_agent = all_agents.back();
        player_agent.id = all_agents.size();
        player_agent.pos = Vec2i(agent.x, agent.y);
        player_agent.color = Color(255, 127, 0);
        player_agent.team = 0;
        player_agent.player_controlled = true;

        view().on_agent_load(player_agent.id, 0, player_agent.pos, player_agent.color);
      }
    }

    current_agent_idx = state.current_agent_idx;
  }

  LevelState level_state() {
    LevelState state;

    state.tiles.resize(tiles.w(), tiles.h());
    for(unsigned int j = 0 ; j < tiles.h() ; j ++) {
      for(unsigned int i = 0 ; i < tiles.w() ; i ++) {
        Vec2i pos(i, j);

        auto type = tiles.get(pos).type;

        TileState tile_state;
        tile_state.type_id = 0;

        if(type == Tile::FLOOR) {
          tile_state.type_id = 0;
        } else if(type == Tile::WALL) {
          tile_state.type_id = 1;
        } else {
          tile_state.type_id = 0;
        }

        state.tiles.set(pos, tile_state);
      }
    }

    for(auto & agent : all_agents) {
      AgentState agent_state;

      agent_state.x = agent.pos.x;
      agent_state.y = agent.pos.y;

      if(agent.player_controlled) {
        agent_state.is_evil = false;
      } else {
        agent_state.is_evil = true;
      }

      state.agents.push_back(agent_state);
    }

    state.current_agent_idx = current_agent_idx;

    return state;
  }

  namespace debug {
    const Agent * get_agent(Id id) {
      for(auto & agent : all_agents) {
        if(agent.id == id) {
          return &agent;
        }
      }
      return nullptr;
    }
  }
}

