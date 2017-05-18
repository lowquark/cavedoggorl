
#include "game.hpp"

#include <game/parts.hpp>
#include <util/Map.hpp>
#include <util/serial.hpp>

#include <cstdio>
#include <fstream>
#include <memory>

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

  // World public
  ObjectHandle World::tick_until_turn(unsigned int max_ticks) {
    for(unsigned int t = 0 ; t < max_ticks ; t ++) {
      auto objs_with_turn = objs_with_turn_this_tick();

      if(!objs_with_turn.empty()) {
        return objs_with_turn.front();
      }

      tick();
    }

    return ObjectHandle();
  }

  // returns true, and the controller id if the entity is player controlled.
  // returns false and an unspecified controller id otherwise
  std::pair<bool, unsigned int> World::is_player_controlled(ObjectHandle obj) const {
    auto player_control_part = get_part<PlayerControlPart>(uni, obj);
    if(player_control_part) {
      return std::pair<bool, unsigned int>(true, player_control_part->player_id);
    }
    return std::pair<bool, unsigned int>(false, 0);
  }
  // executes an automatic turn, defaults to wait in the case of no AI
  void World::ai_turn(ObjectHandle obj) {
    printf("[%s]'s turn\n", obj.str().c_str());

    auto agent_part = get_part<AgentPart>(uni, obj);
    if(agent_part) {
      auto ai_control_part = get_part<AIControlPart>(uni, obj);
      if(ai_control_part) {
        // do something based on ai's desired action
        move_attack_turn(obj, Vec2i(1, 0));
      } else {
        wait_turn(obj);
      }
    }
  }

  void World::wait_turn(ObjectHandle obj) {
  }
  // returns false if there is an obstacle in the way, and it is not
  // destructible
  bool World::move_attack_turn(ObjectHandle obj, Vec2i delta) {
    // find a means of locomotion (just modify spatial position for now)
    auto spatial_part = get_part<SpatialPart>(uni, obj);
    if(spatial_part) {
      Vec2i old_pos = spatial_part->pos;
      Vec2i new_pos = spatial_part->pos + delta;
      spatial_part->pos = new_pos;

      notify_move(obj, old_pos, new_pos);

      auto turn_taker_part = get_part<TurnTakerPart>(uni, obj);
      if(turn_taker_part) {
        turn_taker_part->wait_time += 10;
      }

      return true;
    } else {
      return false;
    }
  }

  // adds a global view
  void World::add_view(View * view) {
    // no nulls
    assert(view);
    // no dups
    for(auto & v : global_views) { if(v == view) { return; } }
    for(auto & v : object_views) { if(v.view == view) { return; } }

    global_views.push_back(view);
  }

  // adds a view with a particular perspective
  void World::add_view(View * view, ObjectHandle obj) {
    // no nulls
    assert(view);
    assert(obj);
    // no dups
    for(auto & v : global_views) { if(v == view) { return; } }
    for(auto & v : object_views) { if(v.view == view) { return; } }

    ObjectView v;
    v.view = view;
    v.object = obj;
    object_views.push_back(v);
    update_view(v);
  }

  void World::set_size(unsigned int w, unsigned int h) {
    tiles.resize(w, h);
    for(auto & v : object_views) {
      v.view->on_world_load(w, h);
    }
    for(auto & v : global_views) {
      v->on_world_load(w, h);
    }
  }
  void World::set_tile(Vec2i pos, unsigned int id) {
    tiles.set(pos, id);
    for(auto & v : object_views) {
      if(v.visible(pos)) {
        v.view->on_tile_load(id, pos);
      }
    }
    for(auto & v : global_views) {
      v->on_tile_load(id, pos);
    }
  }

  void World::create_hero(Vec2i pos) {
    notify_spawn(uni.create_object({
      "Glyph 0 255 127 0",
      SpatialPart::state(pos),
      "TurnTaker 5",
      "Agent",
      "PlayerControl 1",
    }));
  }
  void World::create_badguy(Vec2i pos) {
    notify_spawn(uni.create_object({
      "Glyph 1 200 0 0",
      SpatialPart::state(pos),
      "TurnTaker 10",
      "Agent",
      "AIControl",
    }));
  }

  // World private
  template <typename T>
  static Part * ctor(const std::string & data) {
    return new T(data);
  }
  Part * World::PartFactory::create(const std::string & data) {
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
  void World::PartFactory::destroy(Part * p) {
    delete p;
  }
  World::PartFactory::PartFactory() {
    ctors["Glyph"]         = ctor<GlyphPart>;
    ctors["Spatial"]       = ctor<SpatialPart>;
    ctors["TurnTaker"]     = ctor<TurnTakerPart>;
    ctors["Agent"]         = ctor<AgentPart>;
    ctors["PlayerControl"] = ctor<PlayerControlPart>;
    ctors["AIControl"]     = ctor<AIControlPart>;
  }

  void World::update_view(ObjectView view) {
  }
  void World::update_view(View * view) {
  }

  void World::notify_spawn(ObjectHandle obj) {
    // notify player agents who can see this object
    auto glyph_part = get_part<GlyphPart>(uni, obj);
    if(glyph_part) {
      auto spatial_part = get_part<SpatialPart>(uni, obj);
      if(spatial_part) {
        for(auto & v : object_views) {
          if(v.visible(spatial_part->pos)) {
            v.view->on_agent_load(obj.id(), glyph_part->type_id, spatial_part->pos, glyph_part->color);
          }
        }
        for(auto & v : global_views) {
          v->on_agent_load(obj.id(), glyph_part->type_id, spatial_part->pos, glyph_part->color);
        }
      }
    }
  }
  void World::notify_move(ObjectHandle obj, Vec2i from, Vec2i to) {
    for(auto & v : object_views) {
      if(v.visible(from) || v.visible(to)) {
        v.view->on_agent_move(obj.id(), from, to);
      }
    }
    for(auto & v : global_views) {
      v->on_agent_move(obj.id(), from, to);
    }
  }

  std::vector<ObjectHandle> World::objs_with_turn_this_tick() const {
    std::vector<ObjectHandle> objs_with_turn;

    uni.for_all_with({ TurnTakerPart::part_class }, [&](const Universe & u, ObjectHandle obj) {
      auto part = get_part<TurnTakerPart>(u, obj);
      if(part->wait_time == 0) {
        objs_with_turn.push_back(obj);
      }
    });

    return objs_with_turn;
  }
  void World::tick() {
    uni.for_all_with({ TurnTakerPart::part_class }, [&](Universe & u, ObjectHandle obj) {
      auto part = get_part<TurnTakerPart>(u, obj);
      if(part->wait_time > 0) {
        part->wait_time --;
      }
    });
  }
}

