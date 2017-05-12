
#include "game.hpp"

#include <game/parts.hpp>
#include <util/Map.hpp>
#include <util/serial.hpp>

#include <cstdio>
#include <fstream>

/*
extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
}
*/

namespace game {
  /*
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
  */

  /*
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
  */

  unsigned int take_turn(Universe & u, ObjectHandle obj) {
    printf("[%u]'s turn\n", obj.id());

    return 10;
  }

  struct PartFactory : public BasePartFactory {
    Part * create(const std::string & data) override {
      if(data.substr(0, 5) == "Glyph") {
        return new GlyphPart(data);
      } else if(data.substr(0, 7) == "Spatial") {
        return new SpatialPart(data);
      } else if(data.substr(0, 9) == "TurnTaker") {
        return new TurnTakerPart(data);
      } else if(data.substr(0, 5) == "Agent") {
        return new AgentPart(data);
      }
      return nullptr;
    }
    void destroy(Part * p) override {
      delete p;
    }
  };

  PartFactory factory;
  Universe u(factory);

  std::vector<ObjectHandle> tick_turns;
  bool break_turns = false;
  ObjectHandle break_id = 0;

  unsigned int do_turn() {
    unsigned int elapsed_ticks = 0;
    if(tick_turns.empty()) {
      if(!u.has_any_with({ TurnTakerPart::part_class })) {
        return elapsed_ticks;
      }

      unsigned int min_time = std::numeric_limits<decltype(min_time)>::max();

      u.for_all_with({ TurnTakerPart::part_class }, [&](Universe & u, ObjectHandle obj) { 
        auto part = get_part<TurnTakerPart>(u, obj);

        if(part->wait_time < min_time) {
          min_time = part->wait_time;
        }
      });

      u.for_all_with({ TurnTakerPart::part_class }, [&](Universe & u, ObjectHandle obj) {
        auto part = get_part<TurnTakerPart>(u, obj);

        part->wait_time -= min_time;
        if(part->wait_time == 0) {
          tick_turns.push_back(obj);
        }
      });

      // we will pop from the end of this list, reverse it before then
      std::reverse(tick_turns.begin(), tick_turns.end());

      printf("%lu turns this tick\n", tick_turns.size());

      elapsed_ticks = min_time;
    }

    if(!tick_turns.empty()) {
      ObjectHandle object = tick_turns.back();

      // make sure this object still has a turn taker part
      TurnTakerPart * turn_taker_part = get_part<TurnTakerPart>(u, object);

      if(turn_taker_part) {
        if(turn_taker_part->break_turns) {
          break_turns = true;
          break_id = object;
          return elapsed_ticks;
        } else {
          // the
          tick_turns.pop_back();

          // do the turn!
          unsigned int turn_time = take_turn(u, object);

          turn_taker_part = get_part<TurnTakerPart>(u, object);
          turn_taker_part->wait_time = turn_time;
        }
      }
    }

    return elapsed_ticks;
  }

  ObjectHandle run(unsigned int n_turns) {
    break_turns = false;
    if(n_turns == 0) {
      while(!break_turns) {
        unsigned int elapsed_ticks = do_turn();
        printf("%u\n", elapsed_ticks);
      }
    } else {
      for(int i = 0 ; i < n_turns ; i ++) {
        unsigned int elapsed_ticks = do_turn();
        printf("%u\n", elapsed_ticks);
        if(break_turns) { break; }
      }
    }
    return break_id;
  }


  Id player_id = 0;

  void create_new() {
    u.create_object({ "Glyph", "Spatial 5 5", "TurnTaker 5", "Agent" });
    u.create_object({ "Glyph", "Spatial 5 5", "TurnTaker 10", "Agent" });
  }
  void save(const std::string & name) {
    std::ofstream os(name, std::ofstream::binary);

    if(os.is_open()) {
    }
  }
  void load_old(const std::string & name) {
    std::ifstream is(name, std::ifstream::binary);

    if(is.is_open()) {
    }
  }


  // Player-centric commands
  void move_attack(Vec2i delta) {
    //move_rel(player_id, delta);

    run(10);
  }
  void activate_tile() {
  }
}

