
#include "game.hpp"
#include "Map.hpp"
#include "AStar.hpp"
#include "Log.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace game {
  Log log;

  static View * game_view = nullptr;
  void view(View * view) {
    game_view = view;
  }

  static Map<Tile> tiles(40, 40);

  Agent player_agent;
  static std::vector<Agent> impostor_agents;

  Agent * find_agent(Vec2i pos) {
    for(auto & agent : impostor_agents) {
      if(agent.pos == pos) {
        return &agent;
      }
    }
    if(player_agent.pos == pos) {
      return &player_agent;
    }
    return nullptr;
  }
  bool can_move(Vec2i desired_pos) {
    if(find_agent(desired_pos)) {
      return false;
    }

    return tiles.get(desired_pos).passable;
  }

  void move_attack(Agent & agent, Vec2i dst) {
    Agent * other = find_agent(dst);
    if(other && other->team != agent.team) {
      other->hp -= 10;

      char message[100];
      snprintf(message, sizeof(message), "Ouch! %p lost 10 hp (now at %d)", other, other->hp);
      if(game_view) { game_view->on_message(message); }

      log.logf("Ouch! %p lost 10 hp (now at %d)", other, other->hp);
    } else if(can_move(dst)) {
      if(game_view) { game_view->on_agent_move(agent.id, agent.pos, dst); }
      agent.pos = dst;
    }
  }

  void ai_turn(Agent & agent) {
    Map<unsigned int> tile_costs(tiles.w(), tiles.h());
    for(int y = 0 ; y < tile_costs.h() ; y ++) {
      for(int x = 0 ; x < tile_costs.w() ; x ++) {
        unsigned int cost = tiles.get(Vec2i(x, y)).passable ? 1 : 1000;
        for(auto & agent : impostor_agents) {
          if(agent.pos == Vec2i(x, y)) {
            cost += 4;
          }
        }
        if(player_agent.pos == Vec2i(x, y)) {
          cost += 4;
        }

        tile_costs.set(Vec2i(x, y), cost);
      }
    }

    DoAStar4(agent.path, tile_costs, player_agent.pos, agent.pos);

    if(agent.path.size() > 1) {
      Vec2i next_pos = agent.path[1];
      move_attack(agent, next_pos);
    }

    agent.time = (rand() % 7) + 4;
  }


  void generate_tiles() {
    if(game_view) { game_view->on_world_load(40, 40); }

    Tile wall_tile;
    wall_tile.type = Tile::WALL;
    wall_tile.passable = false;
    Tile floor_tile;
    floor_tile.type = Tile::FLOOR;
    floor_tile.passable = true;

    for(int j = 0 ; j < 40 ; j ++) {
      for(int i = 0 ; i < 40 ; i ++) {
        auto pos = Vec2i(i, j);
        //float dist_from_center = sqrt((j - 20)*(j - 20) + (i - 20)*(i - 20));
        //if(dist_from_center > 20) {
        if((rand() % 4) == 0) {
          tiles.set(pos, wall_tile);
          if(game_view) { game_view->on_tile_load(wall_tile.type, pos); }
        } else {
          tiles.set(pos, floor_tile);
          if(game_view) { game_view->on_tile_load(floor_tile.type, pos); }
        }
      }
    }
  }
  void load_world() {
    srand(time(0));

    generate_tiles();

    player_agent.id = 1;
    player_agent.pos = Vec2i(20, 20);
    player_agent.color = Color(1.0f, 0.5f, 0.0f);
    player_agent.team = 0;

    if(game_view) { game_view->on_agent_load(player_agent.id, 0, player_agent.pos, player_agent.color); }

    impostor_agents.resize(10);
    for(int i = 0 ; i < 10 ; i ++) {
      auto & impostor = impostor_agents[i];
      impostor.id = i + 2;
      impostor.pos = Vec2i(rand() % 40, rand() % 40);
      impostor.color = Color(0.1f, 0.1f, 0.1f) + Color(0.9f * rand() / RAND_MAX,
                                                       0.9f * rand() / RAND_MAX,
                                                       0.9f * rand() / RAND_MAX);
      impostor.team = 1;

      if(game_view) { game_view->on_agent_load(impostor.id, 1, impostor.pos, impostor.color); }
    }
  }

  void step_game() {
    log.logf<Log::DEBUG1>("player turn complete!");
    while(true) {
      for(auto & impostor : impostor_agents) {
        impostor.time --;
        if(impostor.time <= 0) {
          log.logf<Log::DEBUG1>("time for ai turn! %p", &impostor);
          ai_turn(impostor);
          log.logf<Log::DEBUG1>("ai turn complete! %p", &impostor);
        }
      }

      player_agent.time --;
      if(player_agent.time <= 0) {
        log.logf<Log::DEBUG1>("time for player turn!");
        return;
      }
    }
  }

  namespace debug {
    const Agent * get_agent(Id id) {
      for(auto & agent : impostor_agents) {
        if(agent.id == id) {
          return &agent;
        }
      }
      if(player_agent.id == id) {
        return &player_agent;
      }
      return nullptr;
    }
  }
}
