
#include "game.hpp"

#include <game/AStar.hpp>

#include <util/Map.hpp>
#include <util/Log.hpp>

#include <game/events.hpp>
#include <game/properties.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
}

namespace game {
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

  static int lapi_map_size(lua_State * L) {
    unsigned int w = luaL_checkinteger(L, 1);
    unsigned int h = luaL_checkinteger(L, 2);
    tiles.resize(w, h);
    view().on_world_load(w, h);

    return 0;
  }
  static int lapi_load_tile(lua_State * L) {
    Object o = Object::Builder(property_factory)
                .add_property(PROPERTY_TYPE_OBSTRUCTION)
                .build();

    Tile wall_tile;
    wall_tile.type = Tile::WALL;
    wall_tile.passable = false;
    Tile floor_tile;
    floor_tile.type = Tile::FLOOR;
    floor_tile.passable = true;

    Vec2i pos;
    pos.x = luaL_checkinteger(L, 1);
    pos.y = luaL_checkinteger(L, 2);
    int type_id = luaL_checkinteger(L, 3);

    if(type_id == 0) {
      tiles.set(pos, floor_tile);
      view().on_tile_load(floor_tile.type, pos);
    } else if(type_id == 1) {
      tiles.set(pos, wall_tile);
      view().on_tile_load(wall_tile.type, pos);
    }

    return 0;
  }
  static int lapi_place_player(lua_State * L) {
    Vec2i pos;
    pos.x = luaL_checkinteger(L, 1);
    pos.y = luaL_checkinteger(L, 2);

    all_agents.emplace_back();
    auto & player_agent = all_agents.back();
    player_agent.id = all_agents.size();
    player_agent.pos = pos;
    player_agent.color = Color(255, 127, 0);
    player_agent.team = 0;
    player_agent.player_controlled = true;
    view().on_agent_load(player_agent.id, 0, player_agent.pos, player_agent.color);

    return 0;
  }
  static int lapi_place_impostor(lua_State * L) {
    Vec2i pos;
    pos.x = luaL_checkinteger(L, 1);
    pos.y = luaL_checkinteger(L, 2);

    all_agents.emplace_back();
    auto & impostor = all_agents.back();
    impostor.id = all_agents.size();
    impostor.pos = pos;
    impostor.color = Color(50, 50, 50) + Color((float) 205 * rand() / RAND_MAX,
                                               (float) 205 * rand() / RAND_MAX,
                                               (float) 205 * rand() / RAND_MAX);
    impostor.team = 1;

    view().on_agent_load(impostor.id, 1, impostor.pos, impostor.color);

    return 0;
  }

  void step_game() {
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

  void clear_world() {
    tiles.clear();
    all_agents.clear();
    current_agent_idx = 0;
  }

  void new_world() {
    // save_world();
    clear_world();

    lua_State * L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcfunction(L, lapi_load_tile);
    lua_setglobal(L, "load_tile");
    lua_pushcfunction(L, lapi_map_size);
    lua_setglobal(L, "map_size");

    lua_pushcfunction(L, lapi_place_player);
    lua_setglobal(L, "place_player");
    lua_pushcfunction(L, lapi_place_impostor);
    lua_setglobal(L, "place_impostor");

    if(luaL_dofile(L, "generate_world.lua") != 0) {
      printf("%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }

    lua_close(L);
    L = nullptr;

    // make sure it's the player's turn, and everything is in a valid state
    step_game();
  }


  // Player-centric commands
  void move_attack(Vec2i delta) {
    auto & agent = all_agents[current_agent_idx];
    if(agent.player_controlled) {
      move_attack(agent, agent.pos + delta);
    } else {
      log.log<Log::ERROR>("The current agent is not playable.");
    }

    step_game();
  }
  void activate_tile() {
    new_world();
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

