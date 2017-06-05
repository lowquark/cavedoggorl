
#include "game.hpp"

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

  void ViewSys::on_spawn(world::Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);

    auto & e = world.entities.at(eid);

    GlyphQuery q;
    e.ext.query(q);

    View::EntityState es;
    es.glyph_id = q.glyph_id;
    es.pos = e.position;
    for(auto & p : players) {
      p->view().set_entity(eid, es);
    }
  }
  void ViewSys::on_move(world::Id eid, Vec2i from, Vec2i to) {
    printf("%s\n", __PRETTY_FUNCTION__);

    for(auto & p : players) {
      p->view().move_entity(eid, from, to);
    }
  }

  void ViewSys::full_update(Player * player, world::Id eid, world::Id loc) {
    auto & level = world.levels.at(loc);

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

    player->view().set_tiles(tiles);

    for(auto & eid : level.entities) {
      auto & e = world.entities.at(eid);

      GlyphQuery q;
      e.ext.query(q);

      View::EntityState es;
      es.glyph_id = q.glyph_id;
      es.pos = e.position;
      player->view().set_entity(eid, es);
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

  void TurnSys::add_turn(world::Id eid, unsigned int speed) {
    turns[eid].energy = 1000; // ready to roll
    turns[eid].speed = speed;
    printf("Turn added for entity %lu\n", eid);
  }
  void TurnSys::remove_turn(world::Id eid) {
    turns.erase(eid);
    printf("Turn removed for entity %lu\n", eid);
  }
  void TurnSys::tick() {
    for(auto & kvpair : turns) {
      auto & turn = kvpair.second;
      turn.energy += turn.speed;
      printf("%lu: %u, %u\n", kvpair.first, turn.energy, turn.speed);
    }
  }

  std::pair<bool, world::Id> TurnSys::whos_turn() const {
    for(auto & kvpair : turns) {
      auto eid = kvpair.first;
      auto & turn = kvpair.second;

      if(turn.energy >= 1000) {
        return std::pair<bool, world::Id>(true, eid);
      }
    }

    return std::pair<bool, world::Id>(false, world::Id());
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

  void MobSys::move_attack(world::Id eid, Vec2i delta) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto & e = world.entities.at(eid);

    Vec2i old_pos = e.position;
    Vec2i new_pos = e.position + delta;

    e.position = new_pos;
    view_sys.on_move(eid, old_pos, new_pos);
  }

  void MoveAction::perform(MobSys & sys, world::Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    sys.move_attack(eid, delta);
  }
  void WaitAction::perform(MobSys & sys, world::Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
  }

  // Engine public
  Player * Engine::create_player(world::Id id, View & view) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto new_player = new Player(id, view);
    players.push_back(new_player);

    // this is where the motherfucking magic happens

    auto world_player = world_store.player(id);
    auto player_entity = world_store.entity(world_player.entity);

    load_level(player_entity.location);

    view_sys.full_update(new_player, world_player.entity, player_entity.location);

    return new_player;
  }

  std::pair<bool, world::Id> Engine::step(unsigned int max_ticks) {
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
          // if no action to take, yield
          return turn;
        }

        turn = turn_sys.whos_turn();
      }

      tick();
    }

    return std::pair<bool, world::Id>(false, world::Id());
  }
  void Engine::complete_turn(const Action & action) {
    auto turn = turn_sys.whos_turn();
    if(turn.first) {
      action.perform(mob_sys, turn.second);
      turn_sys.finish_turn();
    }
  }
 
  void Engine::set_entity(world::Id eid, const world::Entity & state) {
    auto & e = world.entities[eid];
    if(state.name == "lambdoggo") {
      e.ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 0 255 0 255")));
    } else {
      e.ext.add_part(std::unique_ptr<RandomControlPart>(new RandomControlPart("")));
      e.ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 1 255 0 255")));
    }
    e.name = state.name;
    e.location = state.location;
    e.position = state.position;
    turn_sys.add_turn(eid, 100);
  }

  Level & Engine::load_level(world::Id world_id) {
    world::Level level = world_store.level(world_id);

    // load the level into the space
    auto & new_level = world.levels[world_id];

    new_level.tiles.resize(level.tiles.size());
    new_level.opaque.resize(level.tiles.size());

    for(unsigned int j = 0 ; j < level.tiles.size().y ; j ++) {
      for(unsigned int i = 0 ; i < level.tiles.size().x ; i ++) {
        Vec2i pos(i, j);

        if(level.tiles.get(pos).type_id == 1) {
          new_level.tiles.set(pos, 1);
          new_level.opaque.set(pos, 1);
        } else if(level.tiles.get(pos).type_id == 2) {
          new_level.tiles.set(pos, 2);
          new_level.opaque.set(pos, 0);
        }
      }
    }

    for(auto & eid : level.entities) {
      new_level.entities.insert(eid);
      set_entity(eid, world_store.entity(eid));
    }

    return new_level;
  }
  void Engine::unload_level(world::Id id) {
  }

  // Engine private
  void Engine::tick() {
    turn_sys.tick();
  }
}

