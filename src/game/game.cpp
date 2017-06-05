
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

  void ViewSys::on_spawn(nc::Id eid) {
    printf("%s\n", __PRETTY_FUNCTION__);

    auto e = world.entities[eid];
    if(e) {
      GlyphQuery q;
      e->ext.query(q);

      View::EntityState es;
      es.glyph_id = q.glyph_id;
      es.pos = e->position;
      for(auto & p : players) {
        p->view.set_entity(eid.idx, es);
      }
    }
  }
  void ViewSys::on_move(nc::Id eid, Vec2i from, Vec2i to) {
    printf("%s\n", __PRETTY_FUNCTION__);

    auto e = world.entities[eid];
    if(e) {
      for(auto & p : players) {
        p->view.move_entity(eid.idx, from, to);
      }
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

  /*
  void AISys::add_ai(nc::Id eid) {
    eids.insert(eid);
  }
  void AISys::remove_ai(nc::Id eid) {
    eids.erase(eid);
  }
  void AISys::tick() {
  }
  */

  void TurnSys::add_turn(nc::Id eid, unsigned int speed) {
    turns[eid].energy = 1000; // ready to roll
    turns[eid].speed = speed;
    printf("Turn added for entity %u\n", eid.idx);
  }
  void TurnSys::remove_turn(nc::Id eid) {
    turns.erase(eid);
    printf("Turn removed for entity %u\n", eid.idx);
  }
  void TurnSys::tick() {
    for(auto & kvpair : turns) {
      auto & turn = kvpair.second;
      turn.energy += turn.speed;
      printf("%u: %u, %u\n", kvpair.first.idx, turn.energy, turn.speed);
    }
  }

  std::pair<bool, nc::Id> TurnSys::whos_turn() const {
    for(auto & kvpair : turns) {
      auto eid = kvpair.first;
      auto & turn = kvpair.second;

      if(turn.energy >= 1000) {
        return std::pair<bool, nc::Id>(true, eid);
      }
    }

    return std::pair<bool, nc::Id>(false, nc::Id());
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

  void MobSys::move_attack(nc::Id eid, Vec2i delta) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto e = world.entities[eid];
    if(e) {
      Vec2i old_pos = e->position;
      Vec2i new_pos = e->position + delta;

      e->position = new_pos;
      view_sys.on_move(eid, old_pos, new_pos);
    }
  }

  void MoveAction::perform(MobSys & sys, nc::Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
    sys.move_attack(eid, delta);
  }
  void WaitAction::perform(MobSys & sys, nc::Id eid) const {
    printf("%s\n", __PRETTY_FUNCTION__);
  }

  // Engine public
  Player * Engine::create_player(View & view) {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto new_player = new Player(view);
    players.push_back(new_player);

    // this is where the motherfucking magic happens

    Map<View::TileState> tile_state;
    tile_state.resize(Vec2u(60, 60));
    View::TileState rock;
    rock.glyph_id = 1;
    tile_state.set(Vec2i(0, 0), rock);
    tile_state.set(Vec2i(1, 1), rock);
    tile_state.set(Vec2i(2, 2), rock);
    new_player->view.set_tiles(tile_state);

    View::EntityState es;
    es.glyph_id = 0;
    es.pos = Vec2i(10, 10);
    new_player->view.set_entity(0, es);

    auto eid = world.entities.create();
    auto e = world.entities[eid];
    e->ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 1 255 0 255")));
    e->position = Vec2i(10, 10);

    turn_sys.add_turn(eid, 100);
    view_sys.on_spawn(eid);

    return new_player;
  }

  std::pair<bool, nc::Id> Engine::step(unsigned int max_ticks) {
    for(unsigned int t = 0 ; t < max_ticks ; t ++) {
      auto turn = turn_sys.whos_turn();
      while(turn.first) {
        auto e = world.entities[turn.second];
        if(e) {
          // query for an action to take
          ActionQuery q;
          e->ext.query(q);

          if(q.action) {
            q.action->perform(mob_sys, turn.second);
            turn_sys.finish_turn();
          } else {
            // if no action to take, yield
            return turn;
          }
        } else {
          // this entity is invalid, it should have been deleted
          printf("Warning: Invalid entity still in turn queue\n");
          turn_sys.finish_turn();
        }
        turn = turn_sys.whos_turn();
      }

      tick();
    }

    return std::pair<bool, nc::Id>(false, nc::Id());
  }
  void Engine::complete_turn(const Action & action) {
    auto turn = turn_sys.whos_turn();
    if(turn.first) {
      action.perform(mob_sys, turn.second);
      turn_sys.finish_turn();
    }
  }

  /*
  nc::Id Engine::load_entity(const std::string & type, const std::string & loc, Vec2i pos) {
    auto eid = world.entities.create();
    auto & e = *world.entities[eid];
    if(type == "player_doggo") {
      e.ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 0 255 0 255")));
      turn_sys.add_turn(eid, 100);
    } else {
      e.ext.add_part(std::unique_ptr<GlyphPart>(new GlyphPart("Glyph 1 255 0 255")));
      e.ext.add_part(std::unique_ptr<RandomControlPart>(new RandomControlPart("")));
      turn_sys.add_turn(eid, 100);
      ai_sys.add_ai(eid);
    }
    e.location = loc;
    e.position = pos;

    view_sys.on_spawn(eid);

    return eid;
  }
  */
  /*
  void Engine::unload_entity(nc::Id sid) {
  }
  */
 
  /*
  void Engine::load_space(const std::string & loc, Vec2u size) {
    auto & space = world.spaces[loc];

    space.tiles.resize(size);
    space.opaque.resize(size);
    view_sys.on_load_space(loc);

    for(unsigned int j = 0 ; j < size.y ; j ++) {
      for(unsigned int i = 0 ; i < size.x ; i ++) {
        Vec2i pos(i, j);
        if(rand() % 5 == 0) {
          space.tiles.set(pos, 1);
          space.opaque.set(pos, 1);
          view_sys.on_tile_update(loc, pos, 1);
        } else {
          space.tiles.set(pos, 2);
          space.opaque.set(pos, 0);
          view_sys.on_tile_update(loc, pos, 2);
        }
      }
    }
  }
  */
  /*
  void Engine::unload_space(nc::Id sid) {
  }
  */

  // Engine private
  void Engine::tick() {
    //ai_sys.tick();
    turn_sys.tick();
  }
}

