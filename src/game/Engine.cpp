
#include "Engine.hpp"

#include <game/parts.hpp>
#include <game/AStar.hpp>
#include <util/Map.hpp>
#include <util/serial.hpp>

#include <cstdio>
#include <fstream>
#include <memory>

namespace game {
  bool Level::is_passable(Vec2i pos) {
    return tiles.get(pos) == 2;
  }

  void ViewSys::notify_spawn(Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);

    auto & e = world.entities.at(eid);

    GlyphQuery q;
    e.ext.query(q);

    View::EntityState es;
    es.glyph_id = q.glyph_id;
    es.pos = e.position;

    views.at(0).get().set_entity(eid, es);
  }
  void ViewSys::notify_despawn(Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);

    views.at(0).get().clear_entity(eid);
  }
  void ViewSys::notify_move(Id eid, Vec2i from, Vec2i to) {
    printf("%s\n", __PRETTY_FUNCTION__);

    views.at(0).get().move_entity(eid, from, to);
  }

  void ViewSys::set_view(Id eid, View & view) {
    views.insert(std::make_pair(eid, std::ref(view)));

    auto & entity = world.entities.at(eid);
    auto & level = world.levels.at(entity.location);

    full_update(view, level);
  }
  void ViewSys::unset_view(Id eid) {
    views.erase(eid);
  }

  void ViewSys::full_update(View & view, const Level & level) {
    Map<View::TileState> tiles;
    tiles.resize(level.tiles.size());

    for(unsigned int y = 0 ; y < level.tiles.size().y ; y ++) {
      for(unsigned int x = 0 ; x < level.tiles.size().x ; x ++) {
        Vec2i pos((int)x, (int)y);

        View::TileState tile;
        tile.glyph_id = level.tiles.get(pos);
        tiles.set(pos, tile);
      }
    }

    view.clear_entities();
    view.set_tiles(tiles);

    for(auto & eid : level.entities) {
      auto & e = world.entities.at(eid);

      GlyphQuery q;
      e.ext.query(q);

      View::EntityState es;
      es.glyph_id = q.glyph_id;
      es.pos = e.position;

      view.set_entity(eid, es);
    }
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


  void FOVSys::add(Id eid) {
    entities.insert(eid);
  }
  void FOVSys::remove(Id eid) {
    entities.erase(eid);
  }
  void FOVSys::notify_move(Id eid, Vec2i from, Vec2i to) {
    if(entities.find(eid) != entities.end()) {
      auto & entity = world.entities.at(eid);
      auto & level = world.levels.at(entity.location);

      BresenhamFOV fov;
      fov.update(level.opaque, entity.position, 10);
      entity.fov = fov.sample(Rect2i(Vec2i(), level.tiles.size()));
    }
  }


  void TurnSys::add_turn(Id eid, unsigned int speed) {
    turns[eid].energy = 1000; // ready to roll
    turns[eid].speed = speed;
    printf("Turn added for entity %u\n", eid);
  }
  void TurnSys::remove_turn(Id eid) {
    turns.erase(eid);
    printf("Turn removed for entity %u\n", eid);
  }
  void TurnSys::tick() {
    for(auto & kvpair : turns) {
      auto & turn = kvpair.second;
      turn.energy += turn.speed;
      printf("%u: %u, %u\n", kvpair.first, turn.energy, turn.speed);
    }
  }

  std::pair<bool, Id> TurnSys::whos_turn() const {
    for(auto & kvpair : turns) {
      auto eid = kvpair.first;
      auto & turn = kvpair.second;

      if(turn.energy >= 1000) {
        return std::pair<bool, Id>(true, eid);
      }
    }

    return std::pair<bool, Id>(false, Id());
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

  void MobSys::perform(Id eid, const MoveAction & action) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto & e = world.entities.at(eid);

    Vec2i old_pos = e.position;
    Vec2i new_pos = e.position + action.delta;

    e.position = new_pos;
    view_sys.notify_move(eid, old_pos, new_pos);
  }
  void MobSys::perform(Id eid, const WaitAction & action) {
    printf("%s\n", __PRETTY_FUNCTION__);
  }
  void MobSys::perform(Id eid, const StairAction & action) {
    printf("%s\n", __PRETTY_FUNCTION__);
    //auto & e = world.entities.at(eid);

    //load_sys.transfer_mob(eid, (e.location + 1) % 4, Vec2i(10, 10));
  }

  void MoveAction::perform(MobSys & sys, Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    sys.perform(eid, *this);
  }
  void WaitAction::perform(MobSys & sys, Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    sys.perform(eid, *this);
  }
  void StairAction::perform(MobSys & sys, Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    sys.perform(eid, *this);
  }

  /*
  void LoadSys::transfer_mob(Id eid, Id loc, Vec2i pos) {
    printf("%s\n", __PRETTY_FUNCTION__);

    for(auto & kvpair : world.players) {
      auto pid = kvpair.first;
      auto & p = kvpair.second;

      if(p.entity == eid) {
        printf("%s: Entity is owned by player.\n", __PRETTY_FUNCTION__);

        auto & entity = world.entities.at(p.entity);

        if(entity.location != loc) {
          auto current_level_id = entity.location;
          auto new_level_id = loc;

          auto & current_level = world.levels.at(current_level_id);
          auto & new_level = load_level(new_level_id);

          current_level.entities.erase(eid);
          new_level.entities.insert(eid);

          entity.location = new_level_id;
          entity.position = pos;

          unload_level(current_level_id);

          view_sys.on_transfer(pid, new_level_id);
        }

        return;
      }
    }
  }
  */

  // Engine public
  void Engine::load_entity(Id eid, const std::string & name, Id loc, Vec2i pos) {
    auto & e = world.entities[eid];
    if(name == "lambdoggo") {
      e.ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 0 255 0 255")));
      fov_sys.add(eid);
    } else {
      e.ext.add_part(std::unique_ptr<RandomControlPart>(new RandomControlPart("")));
      e.ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 1 255 0 255")));
    }
    e.name = name;
    e.location = loc;
    e.position = pos;
    turn_sys.add_turn(eid, 100);

    auto & level = world.levels.at(loc);
    level.entities.insert(eid);
  }
  void Engine::unload_entity(Id eid) {
    world.entities.erase(eid);
    fov_sys.remove(eid);
    turn_sys.remove_turn(eid);
  }

  void Engine::load_level(Id loc, const Level & level) {
    world.levels[loc] = level;
  }
  void Engine::unload_level(Id loc) {
    world.levels.erase(loc);
  }

  void Engine::attach_player(Id pid, Id eid, View & view) {
    view_sys.set_view(eid, view);
  }

  Id Engine::step() {
    while(true) {
      auto turn = turn_sys.whos_turn();

      while(turn.first) {
        auto & e = world.entities.at(turn.second);
        // query for an action to take
        ActionQuery q;
        e.ext.query(q);

        if(q.action) {
          q.action->perform(mob_sys, turn.second);
          turn_sys.finish_turn();
        } else {
          // if no action to take, break
          return turn.second;
        }

        turn = turn_sys.whos_turn();
      }

      tick();
    }
  }
  std::pair<Id, bool> Engine::step(unsigned int max_ticks) {
    for(unsigned int t = 0 ; t < max_ticks ; t ++) {
      auto turn = turn_sys.whos_turn();

      while(turn.first) {
        auto & e = world.entities.at(turn.second);
        // query for an action to take
        ActionQuery q;
        e.ext.query(q);

        if(q.action) {
          q.action->perform(mob_sys, turn.second);
          turn_sys.finish_turn();
        } else {
          // if no action to take, ask the player
          return std::make_pair(turn.second, true);
        }

        turn = turn_sys.whos_turn();
      }

      tick();
    }

    return std::make_pair((Id)0, true);
  }
  Id Engine::step(const Action & action) {
    complete_turn(action);
    return step();
  }
  std::pair<Id, bool> Engine::step(const Action & action, unsigned int max_ticks) {
    complete_turn(action);
    return step(max_ticks);
  }

  // Engine private
  void Engine::complete_turn(const Action & action) {
    auto turn = turn_sys.whos_turn();
    if(turn.first) {
      action.perform(mob_sys, turn.second);
      turn_sys.finish_turn();
    }
  }

  void Engine::tick() {
    turn_sys.tick();
  }
}

