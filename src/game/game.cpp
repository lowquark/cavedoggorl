
#include "game.hpp"

#include <util/Map.hpp>

#include <game/Color.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <sstream>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
}

namespace game {
  void step_game() {
    // Step external ai

    step_level();
  }

  LevelState * generated_level_state = nullptr;

  static int lapi_map_size(lua_State * L) {
    unsigned int w = luaL_checkinteger(L, 1);
    unsigned int h = luaL_checkinteger(L, 2);

    generated_level_state->tiles.resize(w, h);

    return 0;
  }
  static int lapi_load_tile(lua_State * L) {
    Vec2i pos;
    pos.x = luaL_checkinteger(L, 1);
    pos.y = luaL_checkinteger(L, 2);
    int type_id = luaL_checkinteger(L, 3);

    TileState tile_state;
    tile_state.type_id = type_id;

    generated_level_state->tiles.set(pos, tile_state);

    return 0;
  }
  static int lapi_place_player(lua_State * L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);

    AgentState agent_state;
    agent_state.x = x;
    agent_state.y = y;
    agent_state.is_evil = false;

    generated_level_state->agents.push_back(agent_state);

    return 0;
  }
  static int lapi_place_impostor(lua_State * L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);

    AgentState agent_state;
    agent_state.x = x;
    agent_state.y = y;
    agent_state.is_evil = true;

    generated_level_state->agents.push_back(agent_state);

    return 0;
  }

  // TODO: Use upvalues in all bound cfunctions
  //
  // Better yet, allow lua to define levels directly (enables generating
  // multiple levels at once)
  bool generate_level(LevelState & state) {
    generated_level_state = &state;

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

    return true;
  }
  bool generate_level(LevelState & state, const std::string & level_name) {
    generate_level(state);

    return true;
  }

  void create_new() {
    LevelState level_state;
    if(generate_level(level_state)) {
      load_level(level_state);

      // make sure it's the player's turn, and everything is in a valid state
      step_game();
    } else {
      log.log<Log::ERROR>("Failed to generate initial level! How can this happen?\n");
    }
  }
  void save(const std::string & name) {
    std::ofstream os(name, std::ofstream::binary);

    if(os.is_open()) {
      LevelState::write(os, level_state());
    }
  }
  void load_old(const std::string & name) {
    std::ifstream is(name, std::ifstream::binary);

    if(is.is_open()) {
      LevelState level_state;
      if(LevelState::read(is, level_state)) {
        load_level(level_state);
      }
    }
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
  }
}

