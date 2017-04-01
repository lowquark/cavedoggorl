
#include <cstdio>
#include <cmath>
#include <cassert>
#include <ctime>

#include <vector>
#include <queue>
#include <map>

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
#include "game.hpp"

/*
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
*/

SDL_Window * window = nullptr;
bool quit_signal = false;


struct MuhView : public game::View {
  struct AgentSprite {
    game::Id type_id = 0;
    Vec2f pos;
    Color color;
  };
  struct TileSprite {
    game::Id type_id = 0;
  };

  class Animation {
    public:
    virtual ~Animation() = default;

    virtual unsigned int type_id() const { return 0; }
    virtual void step() { }
    virtual void start() { }
    virtual void finish() { }
    virtual bool is_finished() const { return true; }
  };
  class AgentMoveAnimation : public Animation {
    AgentSprite & sprite;
    Vec2i from;
    Vec2i to;

    unsigned int t_animation = 4;

    public:
    AgentMoveAnimation(AgentSprite & sprite, const Vec2i & from, const Vec2i & to)
      : sprite(sprite)
      , from(from)
      , to(to) {
    }

    void step() override {
      if(t_animation > 0) {
        t_animation --;
      }
      Vec2f from_f = from;
      Vec2f to_f = to;
      float lerp = (float)t_animation / 5;
      sprite.pos = to_f + (from_f - to_f)*lerp;
    }
    bool is_finished() const override {
      return t_animation == 0;
    }

    void start() override {
      sprite.pos = from;
    }
    void finish() override {
      sprite.pos = to;
    }
  };

  std::deque<Animation *> queued_animations;
  std::vector<Animation *> active_animations;

  Map<TileSprite> tile_sprites;
  std::map<game::Id, AgentSprite> agent_sprites;

  void on_world_load(unsigned int tiles_x, unsigned int tiles_y) override {
    printf("%s: %ux%u\n", __PRETTY_FUNCTION__, tiles_x, tiles_y);
    tile_sprites.resize(tiles_x, tiles_y);
  }
  void on_tile_load(game::Id type_id, const Vec2i & pos) override {
    printf("%s: %lu, (%d, %d)\n", __PRETTY_FUNCTION__, type_id, pos.x, pos.y);
    TileSprite sprite;
    sprite.type_id = type_id;
    tile_sprites.set(pos, sprite);
  }
  void on_agent_load(game::Id agent_id, game::Id type_id, const Vec2i & pos, const Color & color) override {
    printf("%s: %lu, %lu, (%d, %d)\n", __PRETTY_FUNCTION__, agent_id, type_id, pos.x, pos.y);

    AgentSprite sprite;
    sprite.type_id = type_id;
    sprite.pos = pos;
    sprite.color = color;
    agent_sprites[agent_id] = sprite;
  }

  void on_agent_move(game::Id agent_id, const Vec2i & from, const Vec2i & to) override {
    printf("%s: %lu, (%d, %d), (%d, %d)\n", __PRETTY_FUNCTION__, agent_id, from.x, from.y, to.x, to.y);

    auto agent_kvpair_it = agent_sprites.find(agent_id);

    if(agent_kvpair_it != agent_sprites.end()) {
      auto & agent = agent_kvpair_it->second;
      //agent.pos = to;

      active_animations.push_back(new AgentMoveAnimation(agent, from, to));
      active_animations.back()->start();
    }
  }
  void on_agent_death(game::Id agent_id) override {
    printf("%s: %lu\n", __PRETTY_FUNCTION__, agent_id);
  }

  void draw() {
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    auto sprite_kvpair_it = agent_sprites.find(1);
    if(sprite_kvpair_it != agent_sprites.end()) {
      auto & sprite = sprite_kvpair_it->second;
      draw::camera_pos = sprite.pos - Vec2f(TILES_X/2, TILES_Y/2);
    }

    for(int j = 0 ; j < tile_sprites.h() ; j ++) {
      for(int i = 0 ; i < tile_sprites.w() ; i ++) {
        auto pos = Vec2i(i, j);
        auto tile = tile_sprites.get(pos);
        if(tile.type_id == 1) {
          draw::draw_wall(pos);
        } else if(tile.type_id == 2) {
          draw::draw_floor(pos);
        }
      }
    }

    for(auto & kvpairs : agent_sprites) {
      auto & sprite = kvpairs.second;

      // Draw the agent if not hidden
      draw::draw_agent(sprite.pos, sprite.color);

      // Draw the path for debugging
      auto agent = game::debug::get_agent(kvpairs.first);
      if(agent) {
        draw::draw_path(agent->path, agent->color);
      }
    }

    SDL_GL_SwapWindow(window);
  }

  bool is_players_turn() {
    return queued_animations.empty() && active_animations.empty();
  }
  void step_animations() {
    if(active_animations.empty()) {
      if(!queued_animations.empty()) {
        auto anim = queued_animations.front();
        queued_animations.pop_front();
        active_animations.push_back(anim);
      }
    }

    // Purge finished animations
    for(auto it = active_animations.begin() ;
        it != active_animations.end() ; ) {
      auto & anim = *it;
      if(anim->is_finished()) {
        delete anim;
        it = active_animations.erase(it);
      } else {
        it ++;
      }
    }
    // Step remaining animations
    for(auto & anim : active_animations) {
      anim->step();
    }
  }
  void skip_animations() {
    // Purge everything. EVERYTHING.
    for(auto & anim : queued_animations) {
      delete anim;
    }
    queued_animations.clear();

    for(auto & anim : active_animations) {
      // Also finish active animations
      anim->finish();
      delete anim;
    }
    active_animations.clear();
  }
};

MuhView muh_view;
int skip_countdown = 0;

void update() {
  muh_view.step_animations();
  muh_view.draw();

  if(skip_countdown > 0) {
    skip_countdown --;
  }

  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    if(event.type == SDL_QUIT) {
      quit_signal = true;
      return;
    } else if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_0) {
        quit_signal = true;
        return;
      }

      if(muh_view.is_players_turn() && skip_countdown == 0) {
        if(event.key.keysym.sym == SDLK_w) {
          move_attack(game::player_agent, game::player_agent.pos + Vec2i(0, -1));
          game::player_agent.time = (rand() % 5) + 4;
          game::step_game(); // Returns when it's the player's turn
        } else if(event.key.keysym.sym == SDLK_a) {
          move_attack(game::player_agent, game::player_agent.pos + Vec2i(-1, 0));
          game::player_agent.time = (rand() % 5) + 4;
          game::step_game(); // Returns when it's the player's turn
        } else if(event.key.keysym.sym == SDLK_s) {
          move_attack(game::player_agent, game::player_agent.pos + Vec2i(0, 1));
          game::player_agent.time = (rand() % 5) + 4;
          game::step_game(); // Returns when it's the player's turn
        } else if(event.key.keysym.sym == SDLK_d) {
          move_attack(game::player_agent, game::player_agent.pos + Vec2i(1, 0));
          game::player_agent.time = (rand() % 5) + 4;
          game::step_game(); // Returns when it's the player's turn
        }
      } else {
        muh_view.skip_animations();
        skip_countdown = 4;
      }
    }
  }

  SDL_Delay(14);
}

void run() {
  quit_signal = false;
  while(true) {
    update();
    if(quit_signal) { break; }
  }
}

int main(int argc, char ** argv) {
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
      game::view(&muh_view);
      game::load_world();

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

