
#include "game.hpp"

#include <game/parts.hpp>
#include <game/AStar.hpp>
#include <util/Map.hpp>
#include <util/serial.hpp>

#include <cstdio>
#include <fstream>
#include <memory>

namespace game {
  bool Space::is_passable(Vec2i pos) {
    return tiles.get(pos) == 2;
  }

  void ViewSys::on_spawn_space(newcore::Id sid) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto s = world.spaces[sid];
    if(s) {
      view.spawn_space(sid, s->tiles.size());
    }
  }
  void ViewSys::on_despawn_space(newcore::Id sid) {
    view.despawn_space(sid);
  }
  void ViewSys::on_tile_update(newcore::Id sid, Vec2i pos, unsigned int new_val) {
    //printf("%s\n", __PRETTY_FUNCTION__);
    view.set_tile(sid, pos, new_val);
  }

  void ViewSys::on_spawn(newcore::Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto e = world.entities[eid];
    if(e) {
      view.set_glyph(eid, eid.idx == 0 ? 0 : 1, e->position, Color(255, 255, 255));
    }
  }
  void ViewSys::on_despawn(newcore::Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);
    view.clear_glyph(eid);
  }
  void ViewSys::on_move(newcore::Id eid, Vec2i from, Vec2i to) {
    printf("%s\n", __PRETTY_FUNCTION__);
    view.move_glyph(eid, from, to);
  }

  void ViewSys::on_fov_update(newcore::Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);
  }

  /*
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
  */

  void TurnSys::add_turn(newcore::Id eid, unsigned int speed) {
    turns[eid.idx].eid = eid;
    turns[eid.idx].energy = 1000; // ready to roll
    turns[eid.idx].speed = speed;
    printf("Turn added for entity %u\n", eid.idx);
  }
  void TurnSys::remove_turn(newcore::Id eid) {
    turns.erase(eid.idx);
    printf("Turn removed for entity %u\n", eid.idx);
  }
  void TurnSys::tick() {
    for(auto & kvpair : turns) {
      auto & turn = kvpair.second;
      turn.energy += turn.speed;
      printf("%u: %u, %u\n", kvpair.first, turn.energy, turn.speed);
    }
  }

  std::pair<bool, newcore::Id> TurnSys::whos_turn() const {
    for(auto & kvpair : turns) {
      auto & turn = kvpair.second;
      if(turn.energy >= 1000) {
        return std::pair<bool, newcore::Id>(true, turn.eid);
      }
    }
    return std::pair<bool, newcore::Id>(false, newcore::Id());
  }
  void TurnSys::finish_turn() {
    for(auto & kvpair : turns) {
      auto & turn = kvpair.second;
      if(turn.energy >= 1000) {
        turn.energy -= 1000;
        return;
      }
    }
  }

  // Engine public
  std::pair<bool, newcore::Id> Engine::step(unsigned int max_ticks) {
    for(unsigned int t = 0 ; t < max_ticks ; t ++) {
      auto turn = turn_sys.whos_turn();
      if(turn.first) {
        auto e = world.entities[turn.second];
        if(e) {
          // query for an action to take

          // if no action to take, yield
          return turn;
        } else {
          // this entity is invalid, it should have been deleted
          printf("Warning: Invalid entity still in turn queue\n");
          turn_sys.finish_turn();
        }
      }

      tick();
    }

    return std::pair<bool, newcore::Id>(false, newcore::Id());
  }

  void Engine::complete_turn(const Action & action) {
    auto turn = turn_sys.whos_turn();
    if(turn.first) {
      action.perform(*this, turn.second);
      turn_sys.finish_turn();
    }
  }

  newcore::Id Engine::create_hero() {
    auto eid = world.entities.create();
    return eid;
  }
  newcore::Id Engine::create_badguy(newcore::Id kill_eid) {
    auto eid = world.entities.create();
    return eid;
  }
 
  newcore::Id Engine::create_space(Vec2u size) {
    auto sid = world.spaces.create();
    auto & space = *world.spaces[sid];

    space.tiles.resize(size);
    space.opaque.resize(size);
    view_sys.on_spawn_space(sid);

    for(unsigned int j = 0 ; j < size.y ; j ++) {
      for(unsigned int i = 0 ; i < size.x ; i ++) {
        Vec2i pos(i, j);
        if(rand() % 5 == 0) {
          space.tiles.set(pos, 1);
          space.opaque.set(pos, 1);
          view_sys.on_tile_update(sid, pos, 1);
        } else {
          space.tiles.set(pos, 2);
          space.opaque.set(pos, 0);
          view_sys.on_tile_update(sid, pos, 2);
        }
      }
    }

    return sid;
  }

  /*
  void Engine::destroy_space(newcore::Id sid) {
  }
  */

  void Engine::spawn(newcore::Id eid, newcore::Id sid, Vec2i pos) {
    auto e = world.entities[eid];
    auto s = world.spaces[sid];
    if(e && s) {
      if(e->space_id) {
        printf("Already spawned!\n");
      } else {
        turn_sys.add_turn(eid, 100);
        e->position = pos;

        s->entities.insert(eid);
        e->space_id = sid;

        view_sys.on_spawn(eid);
      }
    }
  }

  void Engine::move_attack(newcore::Id eid, Vec2i delta) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto e = world.entities[eid];
    if(e) {
      auto s = world.spaces[e->space_id];
      if(s) {
        Vec2i old_pos = e->position;
        Vec2i new_pos = e->position + delta;

        if(s->is_passable(new_pos)) {
          e->position = new_pos;
          view_sys.on_move(eid, old_pos, new_pos);
        }
      }
    }
  }
  void Engine::wait(newcore::Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);
  }

  // Engine private
  void Engine::tick() {
    turn_sys.tick();
  }

  void MoveAction::perform(Engine & E, newcore::Id eid) const {
    E.move_attack(eid, delta);
  }
  void WaitAction::perform(Engine & E, newcore::Id eid) const {
    E.wait(eid);
  }
}

