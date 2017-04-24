
#include "game.hpp"

#include <util/Map.hpp>
#include <util/serial.hpp>

#include <game/Color.hpp>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <fstream>
#include <algorithm>
#include <sstream>
#include <cassert>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
}

namespace game {
  Level level;

  struct AgentTurnEntry {
    Id agent_id;
    unsigned int time;
  };

  std::vector<AgentTurnEntry> agent_turns;
  unsigned int agent_turn_idx;

  void turn_time(Id agent_id, int time) {
    for(auto & turn : agent_turns) {
      if(turn.agent_id == agent_id) {
        turn.time = time;
      }
    }
  }

  void give_turn(Id agent_id, int initial_time) {
    for(auto & turn : agent_turns) {
      if(turn.agent_id == agent_id) {
        return;
      }
    }

    AgentTurnEntry turn;
    turn.agent_id = agent_id;
    turn.time = initial_time;
    agent_turns.push_back(turn);
  }
  void remove_turn(Id agent_id) {
    for(unsigned int idx = 0 ;
        idx < agent_turns.size() ; ) {
      auto & turn = agent_turns[idx];
      if(turn.agent_id == agent_id) {
        agent_turns.erase(agent_turns.begin() + idx);
        if(agent_turn_idx > idx) {
          agent_turn_idx --;
        }
        return;
      } else {
        idx ++;
      }
    }
  }

  // Returns the id of the agent whose turn is next
  Id next_turn() {
    if(agent_turns.empty()) {
      return 0;
    }

    while(agent_turn_idx < agent_turns.size()) {
      auto & turn = agent_turns[agent_turn_idx];
      if(turn.time == 0) {
        return turn.agent_id;
      }
      agent_turn_idx ++;
    }

    // we've reached the end of the list, time to start over

    // find the soonest turn
    auto soonest_turn_it = std::min_element(agent_turns.begin(), agent_turns.end(),
      [](const AgentTurnEntry & a, const AgentTurnEntry & b) {
        return a.time < b.time;
      }
    );

    // zero out common time
    if(soonest_turn_it->time != 0) {
      for(auto & turn : agent_turns) {
        turn.time -= soonest_turn_it->time;
      }
    }

    agent_turn_idx = soonest_turn_it - agent_turns.begin();
    return soonest_turn_it->agent_id;
  }


  Agent * find_agent(Vec2i pos) {
    return level.get_agent(level.find(pos));
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
    } else if(level.can_move(agent.id, dst)) {
      view().on_agent_move(agent.id, agent.pos, dst);
      agent.pos = dst;
    }

    agent.time = (rand() % 5) + 4;
  }

  void ai_turn(Agent & agent) {
    /*
    Map<unsigned int> tile_costs(level.tiles.w(), level.tiles.h());
    for(int y = 0 ; y < tile_costs.h() ; y ++) {
      for(int x = 0 ; x < tile_costs.w() ; x ++) {
        unsigned int cost = level.tiles.get(Vec2i(x, y)).passable ? 1 : 1000;
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
    */
  }


  bool any_agents_playable() {
    for(auto & agent : level.all_agents) {
      if(agent && agent->player_controlled) {
        return true;
      }
    }
    return false;
  }

  void step_game() {
    // Step external ai

    if(!any_agents_playable()) {
      log.log<Log::ERROR>("Cannot step the game without any player controlled agents.");
      return;
    }

    /*
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
    */
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
      level.load(level_state);

      unsigned int agent_id = 1;
      for(auto & agent_state : level_state.agents) {
        if(agent_state.is_evil) {
        } else {
          give_turn(agent_id, 0);
        }
        agent_id ++;
      }

      // make sure it's the player's turn, and everything is in a valid state
      step_game();
    } else {
      log.log<Log::ERROR>("Failed to generate initial level! How can this happen?\n");
    }
  }
  void save(const std::string & name) {
    std::ofstream os(name, std::ofstream::binary);

    if(os.is_open()) {
      LevelState::write(os, level.state());
    }
  }
  void load_old(const std::string & name) {
    std::ifstream is(name, std::ifstream::binary);

    if(is.is_open()) {
      LevelState level_state;
      if(LevelState::read(is, level_state)) {
        level.load(level_state);
      }
    }
  }


  // Player-centric commands
  void move_attack(Vec2i delta) {
    auto agent_id = next_turn(); 
    auto agent = level.get_agent(agent_id);

    if(agent && agent->player_controlled) {
      move_attack(*agent, agent->pos + delta);
    } else {
      log.logf<Log::ERROR>("The current agent (%u) is not playable.", agent_id);
    }

    step_game();
  }
  void activate_tile() {
  }
}

