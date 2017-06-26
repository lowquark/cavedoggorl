
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


  /*
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
  */


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


  void MoveAction::perform(Engine & eng, Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    eng.perform(*this, eid);
  }
  void WaitAction::perform(Engine & eng, Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    eng.perform(*this, eid);
  }
  void StairAction::perform(Engine & eng, Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    eng.perform(*this, eid);
  }

  // Engine public
  void Engine::load_entity(Id eid, const std::string & name, Id loc, Vec2i pos) {
    auto & e = world.entities[eid];
    if(name == "lambdoggo") {
      e.ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 0 255 0 255")));
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

    obs.notify_entity_load(*this, eid);
  }
  void Engine::unload_entity(Id eid) {
    world.entities.erase(eid);
    turn_sys.remove_turn(eid);

    obs.notify_entity_unload(*this, eid);
  }

  void Engine::load_level(Id loc, const Level & level) {
    world.levels[loc] = level;

    obs.notify_level_load(*this, loc);
  }
  void Engine::unload_level(Id loc) {
    world.levels.erase(loc);

    obs.notify_level_unload(*this, loc);
  }

  void Engine::perform(const Action & action, Id eid) {
    action.perform(*this, eid);
  }
  void Engine::perform(const MoveAction & action, Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto & e = world.entities.at(eid);

    e.position = e.position + action.delta;
    obs.notify_entity_move(*this, eid);
  }
  void Engine::perform(const WaitAction & action, Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);
  }
  void Engine::perform(const StairAction & action, Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);
    //auto & e = world.entities.at(eid);

    //load_sys.transfer_mob(eid, (e.location + 1) % 4, Vec2i(10, 10));
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
          q.action->perform(*this, turn.second);
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
          q.action->perform(*this, turn.second);
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


  Vec2i Engine::entity_position(Id eid) const {
    auto & e = world.entities.at(eid);
    return e.position;
  }
  Id Engine::entity_location(Id eid) const {
    auto & e = world.entities.at(eid);
    return e.location;
  }
  unsigned int Engine::entity_glyph_id(Id eid) const {
    auto & e = world.entities.at(eid);

    GlyphQuery q;
    e.ext.query(q);

    return q.glyph_id;
  }

  Vec2u Engine::level_size(Id lid) const {
    auto & level = world.levels.at(lid);
    return level.tiles.size();
  }
  unsigned int Engine::tile_glyph_id(Id lid, Vec2i pos) const {
    auto & level = world.levels.at(lid);
    return level.tiles.get(pos);
  }
  Map<unsigned int> Engine::tile_glyph_ids(Id lid) const {
    auto & level = world.levels.at(lid);

    /*
    Map<unsigned int> tiles;
    tiles.resize(level.tiles.size());

    for(unsigned int y = 0 ; y < level.tiles.size().y ; y ++) {
      for(unsigned int x = 0 ; x < level.tiles.size().x ; x ++) {
        Vec2i pos((int)x, (int)y);
        unsigned int glyph_id = level.tiles.get(pos);
        tiles.set(pos, glyph_id);
      }
    }

    return tiles;
    */

    return level.tiles;
  }

  // Engine private
  void Engine::complete_turn(const Action & action) {
    auto turn = turn_sys.whos_turn();
    if(turn.first) {
      action.perform(*this, turn.second);
      turn_sys.finish_turn();
    }
  }

  void Engine::tick() {
    turn_sys.tick();
  }
}

