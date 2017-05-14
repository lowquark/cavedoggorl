
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

  // todo: should set per-player id
  static View * _view = nullptr;
  void set_view(View & view) {
    _view = &view;
  }
  // Always valid!
  View & get_view(unsigned int phys_player_id) {
    // todo: establish mapping from id -> view
    static View default_view;
    if(_view) {
      return *_view;
    }
    return default_view;
  }

  void notify_spawn(Universe & u, ObjectHandle obj) {
    // notify player agents who can see this object
    auto glyph_part = get_part<GlyphPart>(u, obj);
    if(glyph_part) {
      auto spatial_part = get_part<SpatialPart>(u, obj);
      if(spatial_part) {
        //u.for_all_with({ AgentPart::part_class, PlayerPart::part_class },
        u.for_all_with({ PlayerPart::part_class },
          [=](Universe & u, ObjectHandle o) {
            auto player_part = get_part<PlayerPart>(u, o);
            auto & v = get_view(player_part->player_id);
            v.on_agent_load(obj.id(), glyph_part->type_id, spatial_part->pos, glyph_part->color);
          }
        );
      }
    }
  }
  void notify_despawn(Universe & u, ObjectHandle obj) {
    // notify player agents who can see this object
    auto glyph_part = get_part<GlyphPart>(u, obj);
    if(glyph_part) {
      //u.for_all_with({ AgentPart::part_class, PlayerPart::part_class },
      u.for_all_with({ PlayerPart::part_class },
        [=](Universe & u, ObjectHandle o) {
          auto player_part = get_part<PlayerPart>(u, o);
          auto & v = get_view(player_part->player_id);
          v.on_agent_death(obj.id());
        }
      );
    }
  }
  void notify_movement(Universe & u, ObjectHandle obj, Vec2i from, Vec2i to) {
    // notify player agents who can see this object
    auto glyph_part = get_part<GlyphPart>(u, obj);
    if(glyph_part) {
      //u.for_all_with({ AgentPart::part_class, PlayerPart::part_class },
      u.for_all_with({ PlayerPart::part_class },
        [=](Universe & u, ObjectHandle o) {
          auto player_part = get_part<PlayerPart>(u, o);
          auto & v = get_view(player_part->player_id);
          v.on_agent_move(obj.id(), from, to);
        }
      );
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

  void take_turn(Universe & u, ObjectHandle obj, TurnTakerPart * turn_taker_part) {
    printf("[%s]'s turn\n", obj.str().c_str());

    auto agent_part = get_part<AgentPart>(u, obj);
    if(agent_part) {
      // do something based on player's desired action
      auto player_part = get_part<PlayerPart>(u, obj);
      if(player_part) {
        player_part->needs_input = true;
        move_agent(u, obj, agent_part, Vec2i(1, 1));
      } else {
        move_agent(u, obj, agent_part, Vec2i(1, 0));
      }
    }

    turn_taker_part->wait_time = 10;
  }

  void owner_look_at(Universe & u, ObjectHandle obj) {
    auto player_part = get_part<PlayerPart>(u, obj);
    if(player_part) {
      get_view(player_part->player_id).look_at(obj.id());
    }
  }

  bool needs_player_input(Universe & u, ObjectHandle obj) {
    auto player_part = get_part<PlayerPart>(u, obj);
    if(player_part) {
      return player_part->needs_input;
    } else {
      return false;
    }
  }

  struct PartFactory : public BasePartFactory {
    std::map<std::string, Part * (*)(const std::string &)> ctors;
    template <typename T>
    static Part * ctor(const std::string & data) {
      return new T(data);
    }
    PartFactory() {
      ctors["Glyph"]     = ctor<GlyphPart>;
      ctors["Spatial"]   = ctor<SpatialPart>;
      ctors["TurnTaker"] = ctor<TurnTakerPart>;
      ctors["Agent"]     = ctor<AgentPart>;
      ctors["Player"]    = ctor<PlayerPart>;
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

  std::vector<ObjectHandle> objs_with_turn;

  void world_tick() {
  }
  ObjectHandle run_until_player_turn(unsigned int max_ticks) {
    unsigned int t = max_ticks;
    while(true) {
      while(!objs_with_turn.empty()) {
        ObjectHandle obj = objs_with_turn.back();

        // does the cached object still have a turn?
        auto turn_taker_part = get_part<TurnTakerPart>(u, obj);
        if(turn_taker_part) {
          // does the object require player action?
          if(needs_player_input(u, obj)) {
            // have it's owner take a look
            owner_look_at(u, obj);
            // we'll try to give this object a turn again on the next call
            return obj;
          } else {
            // no longer pending
            objs_with_turn.pop_back();
            // do the turn!
            take_turn(u, obj, turn_taker_part);
          }
        } else {
          // no longer pending, and no turn
          objs_with_turn.pop_back();
        }
      }
      if(t == 0) {
        return ObjectHandle();
      }

      u.for_all_with({ TurnTakerPart::part_class }, [&](Universe & u, ObjectHandle obj) {
        auto part = get_part<TurnTakerPart>(u, obj);
        if(part->wait_time > 0) {
          part->wait_time --;
        }
      });
      t --;

      world_tick();

      objs_with_turn = objs_with_turn_this_tick();
      std::reverse(objs_with_turn.begin(), objs_with_turn.end());
    }
  }


  void create_new() {
    get_view(1).on_world_load(10, 10);
    for(int j = 0 ; j < 10 ; j ++) {
      for(int i = 0 ; i < 10 ; i ++) {
        get_view(1).on_tile_load(rand() % 2 == 0 ? 1 : 2, Vec2i(i, j));
      }
    }

    // TODO: need to notify of previously spawned objects as well :/
    notify_spawn(u, u.create_object({
      "Glyph 0 255 127 0",
      "Spatial 0 0",
      "TurnTaker 5",
      "Agent",
      "Player 1",
    }));
    notify_spawn(u, u.create_object({
      "Glyph 1 200 0 0",
      "Spatial 1 1",
      "TurnTaker 10",
      "Agent",
    }));
    notify_spawn(u, u.create_object({
      "Glyph 1 55 200 0",
      "Spatial 2 2",
      "TurnTaker 10",
      "Agent",
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


  ObjectHandle player_obj;

  // Player-centric commands
  void move_attack(Vec2i delta) {
    //move_rel(player_id, delta);

    if(player_obj) {
      auto player_part = get_part<PlayerPart>(u, player_obj);
      if(player_part) {
        player_part->needs_input = false;
      }
    }

    player_obj = run_until_player_turn(200);

    printf("player_obj: %s\n", player_obj.str().c_str());
  }
  void activate_tile() {
  }
}

