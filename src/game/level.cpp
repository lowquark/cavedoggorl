
#include "level.hpp"

#include <game/view.hpp>
#include <game/events.hpp>
#include <game/properties.hpp>
#include <game/AStar.hpp>
#include <util/serial.hpp>
#include <iostream>

#include <cstdio>

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


  // state
  void Level::clear() {
    for(auto & agent : all_agents) {
      delete agent;
    }

    tiles.clear();
    all_agents.clear();
    current_agent_idx = 0;
  }
  void Level::load(const LevelState & state) {
    Object o = Object::Builder(property_factory)
                .add_property(PROPERTY_TYPE_OBSTRUCTION)
                .build();

    clear();

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
        auto impostor = new Agent;
        all_agents.push_back(impostor);

        impostor->id = all_agents.size();
        impostor->pos = Vec2i(agent.x, agent.y);
        impostor->color = Color(50, 50, 50) + Color((float) 205 * rand() / RAND_MAX,
                                                    (float) 205 * rand() / RAND_MAX,
                                                    (float) 205 * rand() / RAND_MAX);
        impostor->team = 1;

        view().on_agent_load(impostor->id, 1, impostor->pos, impostor->color);
      } else {
        auto player_agent = new Agent;
        all_agents.push_back(player_agent);

        player_agent->id = all_agents.size();
        player_agent->pos = Vec2i(agent.x, agent.y);
        player_agent->color = Color(255, 127, 0);
        player_agent->team = 0;
        player_agent->player_controlled = true;

        view().on_agent_load(player_agent->id, 0, player_agent->pos, player_agent->color);
      }
    }

    current_agent_idx = state.current_agent_idx;
  }
  LevelState Level::state() {
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
      if(agent) {
        AgentState agent_state;

        agent_state.x = agent->pos.x;
        agent_state.y = agent->pos.y;

        if(agent->player_controlled) {
          agent_state.is_evil = false;
        } else {
          agent_state.is_evil = true;
        }

        state.agents.push_back(agent_state);
      }
    }

    state.current_agent_idx = current_agent_idx;

    return state;
  }

  // queries
  Id Level::find(Vec2i pos) {
    for(auto & agent : all_agents) {
      if(agent && agent->pos == pos) {
        return agent->id;
      }
    }
    return 0;
  }
  bool Level::can_move(Id agent_id, Vec2i desired_pos) {
    if(find(desired_pos)) {
      return false;
    }

    return tiles.get(desired_pos).passable;
  }
  Agent * Level::get_agent(Id agent_id) {
    if(agent_id > 0 && agent_id <= all_agents.size()) {
      return all_agents[agent_id - 1];
    } else {
      return nullptr;
    }
  }

  // actions
  Id Level::create(const Object::Builder & builder, Vec2i pos) {
    return 0;
  }
  bool Level::move(Id agent_id, Vec2i dst) {
    return false;
  }
  void Level::destroy(Id agent_id) {
  }

  Id Level::move(Id src_id, const Level & src, const Level & dst) {
    return 0;
  }

  /*
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
  */
}

