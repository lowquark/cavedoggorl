
#include "game.hpp"

#include <game/AStar.hpp>

#include <util/Map.hpp>
#include <util/Log.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
}

namespace game {
  // Its big, it's heavy, it's wood
  Log log;
  // Better than bad, it's good!

  static View * _view = nullptr;
  void set_view(View & view) {
    _view = &view;
  }
  // Always valid!
  View & view() {
    static View default_view;
    if(_view) {
      return *_view;
    }
    return default_view;
  }

  static Map<Tile> tiles(40, 40);

  // We are all africans
  static std::vector<Agent> all_agents;
  int current_agent_idx = 0;
  bool is_game_over = false;

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

  static int lapi_map_size(lua_State * L) {
    unsigned int w = luaL_checkinteger(L, 1);
    unsigned int h = luaL_checkinteger(L, 2);
    tiles.resize(w, h);
    view().on_world_load(w, h);

    return 0;
  }
  static int lapi_load_tile(lua_State * L) {
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
    player_agent.color = Color(255, 127, 0.0f);
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

  void load_world() {
    is_game_over = false;

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

    if(luaL_dofile(L, "load_world.lua") != 0) {
      printf("%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }

    lua_close(L);
    L = nullptr;
  }

  // Player-centric move attack
  void move_attack(Vec2i delta) {
    if(!is_game_over) {
      auto & agent = all_agents[current_agent_idx];

      move_attack(agent, agent.pos + delta);
    }
  }

  void step_game() {
    if(is_game_over) {
      view().on_message("Stop stepping the game! No more player! Game over!");
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

        is_game_over = true;
        for(auto & agent : all_agents) {
          if(agent.player_controlled) {
            is_game_over = false;
          }
        }

        if(is_game_over) {
          view().on_message("No more player! Game over!");
          return;
        }
      }
    }
  }

  bool is_over() {
    return is_game_over;
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
