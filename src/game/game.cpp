
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


  std::map<unsigned int, View *> player_views;

  void add_player(unsigned int id, View * view) {
    player_views[id] = view;
  }
  void remove_player(unsigned int id) {
    player_views.erase(id);
  }

  // Always valid!
  View * get_view(unsigned int player_id) {
    auto it_kvpair = player_views.find(player_id);
    if(it_kvpair != player_views.end()) {
      return it_kvpair->second;
    } else {
      return nullptr;
    }
  }

  bool visible(Universe & u, ObjectHandle obj, Vec2i pos) {
    return true;
  }
  bool visible(View & view, Vec2i pos) {
    return true;
  }

  void notify_spawn(Universe & u, ObjectHandle obj) {
    // notify player agents who can see this object
    auto glyph_part = get_part<GlyphPart>(u, obj);
    if(glyph_part) {
      auto spatial_part = get_part<SpatialPart>(u, obj);
      if(spatial_part) {
        //u.for_all_with({ AgentPart::part_class, PlayerControlPart::part_class },
        u.for_all_with({ PlayerControlPart::part_class },
          [=](Universe & u, ObjectHandle o) {
            auto player_part = get_part<PlayerControlPart>(u, o);
            auto v = get_view(player_part->player_id);
            if(v) {
              v->on_agent_load(obj.id(), glyph_part->type_id, spatial_part->pos, glyph_part->color);
            }
          }
        );
      }
    }
  }
  void notify_despawn(Universe & u, ObjectHandle obj) {
    // notify player agents who can see this object
    auto glyph_part = get_part<GlyphPart>(u, obj);
    if(glyph_part) {
      //u.for_all_with({ AgentPart::part_class, PlayerControlPart::part_class },
      u.for_all_with({ PlayerControlPart::part_class },
        [=](Universe & u, ObjectHandle o) {
          auto player_part = get_part<PlayerControlPart>(u, o);
          auto v = get_view(player_part->player_id);
          if(v) {
            v->on_agent_death(obj.id());
          }
        }
      );
    }
  }
  void notify_movement(Universe & u, ObjectHandle obj, Vec2i from, Vec2i to) {
    for(auto & kvpair : player_views) {
      auto & view = kvpair.second;
      view->on_agent_move(obj.id(), from, to);
    }
  }


  struct PartFactory : public BasePartFactory {
    std::map<std::string, Part * (*)(const std::string &)> ctors;
    template <typename T>
    static Part * ctor(const std::string & data) {
      return new T(data);
    }
    PartFactory() {
      ctors["Glyph"]         = ctor<GlyphPart>;
      ctors["Spatial"]       = ctor<SpatialPart>;
      ctors["TurnTaker"]     = ctor<TurnTakerPart>;
      ctors["Agent"]         = ctor<AgentPart>;
      ctors["PlayerControl"] = ctor<PlayerControlPart>;
      ctors["AIControl"]     = ctor<AIControlPart>;
    }
    Part * create(const std::string & data) override {
      std::stringstream ss(data);
      std::string name;
      ss >> name;

      auto ctor_it = ctors.find(name);
      if(ctor_it == ctors.end()) {
        return nullptr;
      } else {
        return ctor_it->second(data);
      }
    }
    void destroy(Part * p) override {
      delete p;
    }
  };

  PartFactory factory;
  Universe u(factory);

  void create_new() {
    auto v = get_view(1);
    v->on_world_load(10, 10);
    for(int j = 0 ; j < 10 ; j ++) {
      for(int i = 0 ; i < 10 ; i ++) {
        v->on_tile_load(rand() % 2 == 0 ? 1 : 2, Vec2i(i, j));
      }
    }

    // TODO: need to notify of previously spawned objects as well :/
    notify_spawn(u, u.create_object({
      "Glyph 0 255 127 0",
      "Spatial 0 0",
      "TurnTaker 5",
      "Agent",
      "PlayerControl 1",
    }));
    notify_spawn(u, u.create_object({
      "Glyph 1 200 0 0",
      "Spatial 1 1",
      "TurnTaker 10",
      "Agent",
      "AIControl",
    }));
    notify_spawn(u, u.create_object({
      "Glyph 1 55 200 0",
      "Spatial 2 2",
      "TurnTaker 10",
      "Agent",
      "AIControl",
    }));
    notify_spawn(u, u.create_object({
      "Glyph 1 55 0 200",
      "Spatial 3 3",
      "TurnTaker 10",
      "Agent",
    }));
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

  void move_agent(Universe & u, ObjectHandle obj, AgentPart * agent_part, Vec2i delta) {
    // find a means of locomotion (just modify spatial position for now)
    auto spatial_part = get_part<SpatialPart>(u, obj);
    if(spatial_part) {
      Vec2i old_pos = spatial_part->pos;
      Vec2i new_pos = spatial_part->pos + delta;
      spatial_part->pos = new_pos;

      notify_movement(u, obj, old_pos, new_pos);
    }
  }

  void ai_turn(Universe & u, ObjectHandle obj, TurnTakerPart * turn_taker_part) {
    printf("[%s]'s turn\n", obj.str().c_str());

    auto agent_part = get_part<AgentPart>(u, obj);
    if(agent_part) {
      auto ai_control_part = get_part<AIControlPart>(u, obj);
      if(ai_control_part) {
        // do something based on agent's desired action
        move_agent(u, obj, agent_part, Vec2i(1, 0));
        turn_taker_part->wait_time = 10;
      } else {
        turn_taker_part->wait_time = 5;
      }
    }
  }

  void owner_look_at(Universe & u, ObjectHandle obj) {
    auto player_part = get_part<PlayerControlPart>(u, obj);
    if(player_part) {
      auto v = get_view(player_part->player_id);
      if(v) {
        v->look_at(obj.id());
      }
    }
  }

  std::vector<ObjectHandle> objs_with_turn_this_tick() {
    std::vector<ObjectHandle> objs_with_turn;

    u.for_all_with({ TurnTakerPart::part_class }, [&](Universe & u, ObjectHandle obj) {
      auto part = get_part<TurnTakerPart>(u, obj);
      if(part->wait_time == 0) {
        objs_with_turn.push_back(obj);
      }
    });

    return objs_with_turn;
  }

  void world_tick() {
  }
  ObjectHandle step_until_player_turn(unsigned int max_ticks) {
    for(unsigned int t = 0 ; t < max_ticks ; t ++) {
      auto objs_with_turn = objs_with_turn_this_tick();

      for(auto & obj : objs_with_turn) {
        if(get_part<PlayerControlPart>(u, obj)) {
          // this object requires a manual turn be taken -- have it's owner take a look
          owner_look_at(u, obj);
          return obj;
        } else {
          // ai turn!
          ai_turn(u, obj, get_part<TurnTakerPart>(u, obj));
        }
      }

      u.for_all_with({ TurnTakerPart::part_class }, [&](Universe & u, ObjectHandle obj) {
        auto part = get_part<TurnTakerPart>(u, obj);
        if(part->wait_time > 0) {
          part->wait_time --;
        }
      });

      world_tick();
    }

    return ObjectHandle();
  }

  void move_attack(ObjectHandle obj, Vec2i delta) {
    auto agent_part = get_part<AgentPart>(u, obj);
    if(agent_part) {
      // do something based on agent's desired action
      move_agent(u, obj, agent_part, delta);
    }

    auto turn_taker_part = get_part<TurnTakerPart>(u, obj);
    if(turn_taker_part) {
      turn_taker_part->wait_time = 10;
    }
  }
}

