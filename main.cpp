
#include <cstdio>
#include <cmath>
#include <cassert>
#include <ctime>

#include <vector>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "Color.hpp"
#include "Vec2.hpp"
#include "Map.hpp"
#include "AStar.hpp"
#include "Entity.hpp"
#include "draw.hpp"

namespace mytime {
  static uint32_t _millis() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return (((spec.tv_sec*1000) & 0xFFFFFFFFu) + spec.tv_nsec/1000000) & 0xFFFFFFFFu;
  }
  static uint32_t _millis_cpu() {
    struct timespec spec;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &spec);
    return (((spec.tv_sec*1000) & 0xFFFFFFFFu) + spec.tv_nsec/1000000) & 0xFFFFFFFFu;
  }

  static uint32_t initial_millis = 0;
  static uint32_t initial_millis_cpu = 0;

  void init() {
    initial_millis = _millis();
    initial_millis_cpu = _millis_cpu();
  }

  uint32_t millis() {
    return _millis() - initial_millis;
  }
  uint32_t millis_cpu() {
    return _millis_cpu() - initial_millis_cpu;
  }

  int32_t diff(uint32_t a, uint32_t b) {
    return (int32_t)(a - b);
  }
}

SDL_Window * window = nullptr;
bool quit_signal = false;

struct Tile {
  enum Type { EMPTY, WALL, FLOOR };
  Type type = EMPTY;
  bool passable = false;
};
Map<Tile> tiles(40, 40);

struct Action;

struct Agent {
  // Game mechanics
  Vec2i pos;

  unsigned int speed;
  int team = -1;

  int hp_max = 100;
  int hp = 100;
  bool is_dead() const { return hp <= 0; }

  int time = 0;

  // Rendering
  Color color;

  // AI
  std::vector<Vec2i> path;
  const Action & next_action() const;

  Agent() : color(1.0f, 0.0f, 1.0f) {}
};

Agent player_agent;
std::vector<Agent> impostor_agents;

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
    printf("Ouch! %p lost 10 hp (now at %d)\n", other, other->hp);
  } else if(can_move(dst)) {
    agent.pos = dst;
  }
}

/*
struct AgentAction {
  virtual unsigned int perform(Agent & agent) const { return 1; };
};

struct AgentMoveAttackAction : public AgentAction {
  Vec2i dst;
  unsigned int perform(Agent & agent) const override {
    printf("perform! %d, %d\n", dst.x, dst.y);
    move_attack(agent, dst);
    return 1;
  };
};
*/

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

  agent.time = (rand() % 10) + 4;
}


void generate_tiles() {
  Tile wall_tile;
  wall_tile.type = Tile::WALL;
  wall_tile.passable = false;
  Tile floor_tile;
  floor_tile.type = Tile::FLOOR;
  floor_tile.passable = true;

  for(int j = 0 ; j < 40 ; j ++) {
    for(int i = 0 ; i < 40 ; i ++) {
      float dist_from_center = sqrt((j - 20)*(j - 20) + (i - 20)*(i - 20));
      if(dist_from_center > 20) {
        tiles.set(Vec2i(i, j), wall_tile);
      } else {
        tiles.set(Vec2i(i, j), floor_tile);
      }
    }
  }
}
void load_world() {
  srand(time(0));

  generate_tiles();

  player_agent.pos = Vec2i(20, 20);
  player_agent.color = Color(1.0f, 0.5f, 0.0f);
  player_agent.team = 0;

  impostor_agents.resize(10);
  for(int i = 0 ; i < 10 ; i ++) {
    impostor_agents[i].pos = Vec2i(rand() % 40, rand() % 40);
    impostor_agents[i].color = Color(0.1f, 0.1f, 0.1f) + Color(0.9f * rand() / RAND_MAX,
                                                         0.9f * rand() / RAND_MAX,
                                                         0.9f * rand() / RAND_MAX);
    impostor_agents[i].team = 1;
  }
}


void draw_world() {
  glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  draw::camera_pos = player_agent.pos - Vec2i(TILES_X/2, TILES_Y/2);

  for(int j = 0 ; j < tiles.h() ; j ++) {
    for(int i = 0 ; i < tiles.w() ; i ++) {
      if(tiles.get(Vec2i(i, j)).type == Tile::WALL) {
        draw::draw_wall(Vec2i(i, j));
      } else if(tiles.get(Vec2i(i, j)).type == Tile::FLOOR) {
        draw::draw_floor(Vec2i(i, j));
      }
    }
  }

  draw::draw_agent(player_agent.pos, player_agent.color);

  for(auto & agent : impostor_agents) {
    draw::draw_agent(agent.pos, agent.color);
    draw::draw_path(agent.path, agent.color);
  }

  SDL_GL_SwapWindow(window);
}


void step_game() {
  printf("player turn complete!\n");
  while(true) {
    for(auto & impostor : impostor_agents) {
      impostor.time --;
      if(impostor.time <= 0) {
        printf("time for ai turn! %p\n", &impostor);
        ai_turn(impostor);
        printf("ai turn complete! %p\n", &impostor);
      }
    }

    player_agent.time --;
    if(player_agent.time <= 0) {
      printf("time for player turn!\n");
      return;
    }
  }
}


void update() {
  draw_world();

  SDL_Event event;
  if(SDL_WaitEvent(&event)) {
    if(event.type == SDL_QUIT) {
      quit_signal = true;
      return;
    } else if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_0) {
        quit_signal = true;
        return;
      }

      if(event.key.keysym.sym == SDLK_w) {
        move_attack(player_agent, player_agent.pos + Vec2i(0, -1));
        player_agent.time = (rand() % 5) + 4;
        step_game();
      } else if(event.key.keysym.sym == SDLK_a) {
        move_attack(player_agent, player_agent.pos + Vec2i(-1, 0));
        player_agent.time = (rand() % 5) + 4;
        step_game();
      } else if(event.key.keysym.sym == SDLK_s) {
        move_attack(player_agent, player_agent.pos + Vec2i(0, 1));
        player_agent.time = (rand() % 5) + 4;
        step_game();
      } else if(event.key.keysym.sym == SDLK_d) {
        move_attack(player_agent, player_agent.pos + Vec2i(1, 0));
        player_agent.time = (rand() % 5) + 4;
        step_game();
      }
    }
  }
}

void run() {
  quit_signal = false;
  while(true) {
    update();
    if(quit_signal) { break; }
  }
}

int main(int argc, char ** argv) {
  mytime::init();

  SDL_Init(SDL_INIT_VIDEO);

  window = SDL_CreateWindow(
      "mvp",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      WINDOW_WIDTH,
      WINDOW_HEIGHT,
      SDL_WINDOW_OPENGL);

  if(window != nullptr) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

    if(gl_ctx != nullptr) {
      load_world();

      run();

      SDL_GL_DeleteContext(gl_ctx);
      gl_ctx = nullptr;
    } else {
      fprintf(stderr, "Failed to create GL context: %s\n", SDL_GetError());
    }

    SDL_DestroyWindow(window);
    window = nullptr;
  } else {
    fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
  }

  SDL_Quit();

  return 0;
}

