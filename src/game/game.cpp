
#include "game.hpp"

#include <game/parts.hpp>
#include <game/AStar.hpp>
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
  // adds and initializes a global view
  void ViewSys::set_view(View * view) {
    this->view = view;
    if(view) {
      view->clear();
      view->set_world_size(space.tiles.w(), space.tiles.h());
      for(unsigned int j = 0 ; j < space.tiles.h() ; j ++) {
        for(unsigned int i = 0 ; i < space.tiles.w() ; i ++) {
          Vec2i pos(i, j);
          view->set_tile(pos, space.tiles.get(pos));
        }
      }
      uni.for_all_with({ GlyphPart::part_class, SpatialPart::part_class },
        [=](const Universe & u, ObjectHandle obj) {
          auto glyph_part = get_part<GlyphPart>(u, obj);
          auto spatial_part = get_part<SpatialPart>(u, obj);
          view->set_glyph(obj.id(), glyph_part->type_id, spatial_part->pos, glyph_part->color);
        }
      );
      uni.for_all_with({ AgentPart::part_class },
        [=](const Universe & u, ObjectHandle obj) {
          auto agent_part = get_part<AgentPart>(u, obj);
          view->set_fov(obj.id(), agent_part->fov);
        }
      );
    }
  }
  */

  void ViewSys::on_world_resize(unsigned int w, unsigned int h) {
    printf("%s\n", __PRETTY_FUNCTION__);
    view.set_world_size(w, h);
  }
  void ViewSys::on_tile_update(Vec2i pos, unsigned int new_val) {
    //printf("%s\n", __PRETTY_FUNCTION__);
    view.set_tile(pos, new_val);
  }

  void ViewSys::on_spawn(ObjectHandle obj) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto glyph_part = get_part<GlyphPart>(uni, obj);
    auto spatial_part = get_part<SpatialPart>(uni, obj);
    if(glyph_part && spatial_part) {
      view.set_glyph(obj.id(), glyph_part->type_id, spatial_part->pos, glyph_part->color);
    }
  }
  void ViewSys::on_despawn(ObjectHandle obj) {
    printf("%s\n", __PRETTY_FUNCTION__);
  }
  void ViewSys::on_move(ObjectHandle obj, Vec2i from, Vec2i to) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto glyph_part = get_part<GlyphPart>(uni, obj);
    auto spatial_part = get_part<SpatialPart>(uni, obj);
    if(glyph_part && spatial_part) {
      view.move_glyph(obj.id(), from, to);
    }
  }

  void ViewSys::on_fov_update(ObjectHandle obj) {
    printf("%s\n", __PRETTY_FUNCTION__);

    auto agent_part = get_part<AgentPart>(uni, obj);

    if(agent_part) {
      view.set_fov(obj.id(), agent_part->fov);
    }
  }


  void FOVSys::on_spawn(ObjectHandle obj) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto agent_part = get_part<AgentPart>(uni, obj);
    auto spatial_part = get_part<SpatialPart>(uni, obj);

    if(agent_part && spatial_part) {
      agent_part->fov.update(space.opaque, spatial_part->pos, 10);

      for(auto & h : fov_update_handlers) {
        h->on_fov_update(obj);
      }
    }
  }
  void FOVSys::on_move(ObjectHandle obj, Vec2i from, Vec2i to) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto agent_part = get_part<AgentPart>(uni, obj);
    auto spatial_part = get_part<SpatialPart>(uni, obj);

    if(agent_part && spatial_part) {
      agent_part->fov.update(space.opaque, spatial_part->pos, 10);

      for(auto & h : fov_update_handlers) {
        h->on_fov_update(obj);
      }

      /*
      for(int j = 0 ; j < 21 ; j ++) {
        for(int i = 0 ; i < 21 ; i ++) {
          Vec2i p(i, j);
          if(agent_part->fov.is_visible(p)) {
            if(p == spatial_part->pos) {
              printf("@");
            } else if(space.opaque.get(p) == 1) {
              printf("#");
            } else {
              printf(".");
            }
          } else {
            printf(" ");
          }
        }
        printf("\n");
      }
      printf("\n");
      */
    }
  }


  unsigned int PhysicsSys::move(ObjectHandle obj, Vec2i delta) {
    printf("%s, %d, %d\n", __PRETTY_FUNCTION__, delta.x, delta.y);

    // find a means of locomotion (just modify spatial position for now)
    auto spatial_part = get_part<SpatialPart>(uni, obj);
    if(spatial_part) {
      Vec2i old_pos = spatial_part->pos;
      Vec2i new_pos = spatial_part->pos + delta;

      if(is_passable(new_pos)) {
        spatial_part->pos = new_pos;

        notify_move(obj, old_pos, new_pos);
      }
    }

    return 10;
  }
  bool PhysicsSys::is_passable(Vec2i pos) {
    bool obstructed = uni.has_any_with({ SpatialPart::part_class },
      [=](const Universe & u, ObjectHandle obj) {
        auto spatial_part = get_part<SpatialPart>(u, obj);
        return spatial_part->pos == pos;
      }
    );
    return !obstructed && space.tiles.get(pos) == 2;
  }
  std::vector<Vec2i> PhysicsSys::find_path(Vec2i from, Vec2i to) {
    Map<unsigned int> cost_map(space.tiles.w(), space.tiles.h());

    for(unsigned int y = 0 ; y < space.tiles.h() ; y ++) {
      for(unsigned int x = 0 ; x < space.tiles.w() ; x ++) {
        Vec2i pos(x, y);
        if(is_passable(pos)) {
          cost_map.set(pos, 0);
        } else {
          cost_map.set(pos, 100);
        }
      }
    }

    std::vector<Vec2i> path;

    DoAStar8(path, cost_map, from, to);

    return path;
  }

  void PhysicsSys::notify_move(ObjectHandle obj, Vec2i from, Vec2i to) {
    for(auto & h : move_handlers) {
      h->on_move(obj, from, to);
    }
  }

  unsigned int AISys::on_turn(ObjectHandle obj) {
    printf("%s\n", __PRETTY_FUNCTION__);

    unsigned int wait_time = 10;

    auto agent_part = get_part<AgentPart>(uni, obj);
    if(agent_part) {
      auto ai_control_part = get_part<AIControlPart>(uni, obj);
      auto spatial_part = get_part<SpatialPart>(uni, obj);
      if(ai_control_part && spatial_part) {
        // do something based on ai's desired action
        auto spatial_part_kill = get_part<SpatialPart>(uni, ai_control_part->kill_obj);

        if(spatial_part_kill) {
          // search and destroy
          ai_control_part->path = phys_sys.find_path(spatial_part->pos, spatial_part_kill->pos);

          if(ai_control_part->path.size() > 1) {
            return phys_sys.move(obj, *(ai_control_part->path.end() - 2) - spatial_part->pos);
          }
        } else {
          // "wander"
        }
      }
    }

    return wait_time;
  }

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

  template <typename T>
  static newcore::Part * part_ctor(const std::string & data) {
    return new T(data);
  }
  EntityManager::EntityManager() {
    part_ctors["Phys"]          = part_ctor<PhysPart>;
    part_ctors["Glyph"]         = part_ctor<GlyphPart>;
    part_ctors["Spatial"]       = part_ctor<SpatialPart>;
    part_ctors["TurnTaker"]     = part_ctor<TurnTakerPart>;
    part_ctors["Agent"]         = part_ctor<AgentPart>;
    part_ctors["PlayerControl"] = part_ctor<PlayerControlPart>;
    part_ctors["AIControl"]     = part_ctor<AIControlPart>;
  }
  newcore::Part * EntityManager::create_part(newcore::EntityId id, const std::string & data) {
    std::stringstream ss(data);
    std::string name;
    ss >> name;

    auto ctor_it = part_ctors.find(name);
    if(ctor_it == part_ctors.end()) {
      return nullptr;
    } else {
      return ctor_it->second(data);
    }
  }
  void EntityManager::destroy_part(newcore::EntityId id, newcore::Part * p) {
    delete p;
  }

  // World public
  std::pair<bool, ObjectHandle> Engine::step(unsigned int max_ticks) {
    for(unsigned int t = 0 ; t < max_ticks ; t ++) {
      auto objs_with_turn = objs_with_turn_this_tick();

      if(!objs_with_turn.empty()) {
        auto obj = objs_with_turn.front();

        auto player_control_part = get_part<PlayerControlPart>(uni, obj);
        if(player_control_part) {
          incomplete_turn = obj;
          return std::pair<bool, ObjectHandle>(true, obj);
        } else {
          auto turn_taker_part = get_part<TurnTakerPart>(uni, obj);
          //ai_turn(obj);
          turn_taker_part->wait_time = ai_sys.on_turn(obj);
        }
      }

      tick();
    }

    incomplete_turn = ObjectHandle();
    return std::pair<bool, ObjectHandle>(false, ObjectHandle());
  }

  void Engine::complete_turn(const Action & action) {
    if(incomplete_turn) {
      action.perform(*this, incomplete_turn);
    }
  }

  void Engine::move_attack(ObjectHandle obj, Vec2i delta) {
    if(obj) {
      auto turn_taker_part = get_part<TurnTakerPart>(uni, obj);
      if(turn_taker_part) {
        turn_taker_part->wait_time = phys_sys.move(obj, delta);
      }
    }
  }
  void Engine::wait(ObjectHandle obj) {
    if(obj) {
      auto turn_taker_part = get_part<TurnTakerPart>(uni, obj);
      if(turn_taker_part) {
        turn_taker_part->wait_time = 10;
      }
    }
  }

  unsigned int Engine::create_space(Vec2u size) {
    space.tiles.resize(size.x, size.y);
    space.opaque.resize(size.x, size.y);
    view_sys.on_world_resize(size.x, size.y);

    for(unsigned int j = 0 ; j < size.y ; j ++) {
      for(unsigned int i = 0 ; i < size.x ; i ++) {
        Vec2i pos(i, j);
        if(rand() % 5 == 0) {
          space.tiles.set(pos, 1);
          space.opaque.set(pos, 1);
          view_sys.on_tile_update(pos, 1);
        } else {
          space.tiles.set(pos, 2);
          space.opaque.set(pos, 0);
          view_sys.on_tile_update(pos, 2);
        }
      }
    }

    return 0;
  }
  void Engine::destroy_space(unsigned int sid) {
  }

  ObjectHandle Engine::create_hero() {
    auto obj = uni.create_object({
      "Glyph 0 255 127 0",
      "TurnTaker 5",
      "Agent",
      "PlayerControl 1",
    });
    return obj;
  };
  ObjectHandle Engine::create_badguy(ObjectHandle kill_obj) {
    auto obj = uni.create_object({
      "Glyph 1 200 0 0",
      "TurnTaker 10",
      "Agent",
      AIControlPart::state(kill_obj),
    });
    return obj;
  }
  void Engine::spawn(ObjectHandle obj, unsigned int sid, Vec2i pos) {
    for(auto & a : space.agents) {
      if(a == obj) {
        return;
      }
    }
    space.agents.push_back(obj);
    uni.create_part(obj, SpatialPart::state(pos));

    view_sys.on_spawn(obj);
    fov_sys.on_spawn(obj);
  }

  // Engine private
  template <typename T>
  static Part * ctor(const std::string & data) {
    return new T(data);
  }
  Engine::PartFactory::PartFactory() {
    ctors["Glyph"]         = ctor<GlyphPart>;
    ctors["Spatial"]       = ctor<SpatialPart>;
    ctors["TurnTaker"]     = ctor<TurnTakerPart>;
    ctors["Agent"]         = ctor<AgentPart>;
    ctors["PlayerControl"] = ctor<PlayerControlPart>;
    ctors["AIControl"]     = ctor<AIControlPart>;
  }
  Part * Engine::PartFactory::create(const std::string & data) {
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
  void Engine::PartFactory::destroy(Part * p) {
    delete p;
  }

  ObjectHandle Engine::get_player() {
    auto objs_with_turn = objs_with_turn_this_tick();

    if(!objs_with_turn.empty()) {
      auto obj = objs_with_turn.front();

      auto player_control_part = get_part<PlayerControlPart>(uni, obj);
      if(player_control_part) {
        return obj;
      }
    }

    return ObjectHandle();
  }

  std::vector<ObjectHandle> Engine::objs_with_turn_this_tick() const {
    std::vector<ObjectHandle> objs_with_turn;

    uni.for_all_with({ TurnTakerPart::part_class }, [&](const Universe & u, ObjectHandle obj) {
      auto part = get_part<TurnTakerPart>(u, obj);
      if(part->wait_time == 0) {
        objs_with_turn.push_back(obj);
      }
    });

    return objs_with_turn;
  }
  void Engine::tick() {
    uni.for_all_with({ TurnTakerPart::part_class }, [&](Universe & u, ObjectHandle obj) {
      auto part = get_part<TurnTakerPart>(u, obj);
      if(part->wait_time > 0) {
        part->wait_time --;
      }
    });
  }

  void MoveAction::perform(Engine & E, ObjectHandle obj) const {
    E.move_attack(obj, delta);
  }
  void WaitAction::perform(Engine & E, ObjectHandle obj) const {
    E.wait(obj);
  }
}

