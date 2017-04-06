
#include <cstdio>
#include <cmath>
#include <cassert>
#include <ctime>

#include <vector>
#include <queue>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "Vec2.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "Log.hpp"


static std::string original_message = "They're going to eat you!\nDon't just stand there! --RUN!!";

static constexpr int TILES_X = 30;
static constexpr int TILES_Y = 24;

static const Vec2u tile_size(32, 32);
static const Vec2u window_size(tile_size.x * TILES_X, tile_size.y * TILES_Y);


SDL_Window * window = nullptr;
bool quit_signal = false;


gfx::GridWorld grid_world;
gfx::HUDOverlay hud;
gfx::WorldMessageLog message_log;
int skip_countdown = 0;


class MuhView : public game::View {
  void on_world_load(unsigned int tiles_x, unsigned int tiles_y) override {
    grid_world.set_size(Vec2u(tiles_x, tiles_y));
  }
  void on_tile_load(game::Id type_id, const Vec2i & pos) override {
    grid_world.set_tile(pos, type_id);
  }
  void on_agent_load(game::Id agent_id, game::Id type_id, const Vec2i & pos, const Color & color) override {
    grid_world.add_agent(agent_id, type_id, pos, color);
  }

  void on_agent_move(game::Id agent_id, const Vec2i & from, const Vec2i & to) override {
    grid_world.move_agent(agent_id, from, to);
  }
  void on_agent_death(game::Id agent_id) override {
  }

  void on_message(const std::string & message) override {
    message_log.push(message);
  }
};

MuhView muh_view;


Vec2i mouse_tile;

void update() {
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

      grid_world.skip_animations();

      if(grid_world.are_animations_finished()) {
        if(skip_countdown == 0) {
          if(event.key.keysym.sym == SDLK_w) {
            game::move_attack(game::player_agent, game::player_agent.pos + Vec2i(0, -1));
            game::player_agent.time = (rand() % 5) + 4;
            game::step_game(); // Returns when it's the player's turn
          } else if(event.key.keysym.sym == SDLK_a) {
            game::move_attack(game::player_agent, game::player_agent.pos + Vec2i(-1, 0));
            game::player_agent.time = (rand() % 5) + 4;
            game::step_game(); // Returns when it's the player's turn
          } else if(event.key.keysym.sym == SDLK_s) {
            game::move_attack(game::player_agent, game::player_agent.pos + Vec2i(0, 1));
            game::player_agent.time = (rand() % 5) + 4;
            game::step_game(); // Returns when it's the player's turn
          } else if(event.key.keysym.sym == SDLK_d) {
            game::move_attack(game::player_agent, game::player_agent.pos + Vec2i(1, 0));
            game::player_agent.time = (rand() % 5) + 4;
            game::step_game(); // Returns when it's the player's turn
          }
        }
      } else {
        grid_world.skip_animations();
        skip_countdown = 2;
      }
    }
  }

  Vec2i mouse_pos;
  SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

  Vec2i new_mouse_tile = grid_world.grid_pos(mouse_pos - grid_world.draw_rect().pos);
  if(new_mouse_tile != mouse_tile) {
    mouse_tile = new_mouse_tile;

    std::string look_str;
    if(grid_world.look_str(look_str, mouse_tile)) {
      look_str = std::string("You see: ") + look_str;
      hud.look(grid_world.draw_rect().pos + grid_world.screen_pos(mouse_tile), look_str);
    } else {
      hud.look_finish();
    }
  }


  grid_world.tick();
  hud.tick();
  message_log.tick();


  glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0f, window_size.x, window_size.y, 0.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_STENCIL_TEST);


  grid_world.draw();
  hud.draw();
  message_log.draw();


  SDL_GL_SwapWindow(window);

  SDL_Delay(14);
}

void run() {
  quit_signal = false;
  while(true) {
    update();
    if(quit_signal) { break; }
  }
}

Log main_log;

int main(int argc, char ** argv) {
  // I hate these
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  const char * font = "/usr/share/fonts/TTF/DejaVuSans.ttf";
  main_log.logf("Loading font from %s...", font);
  main_log.flush();

  gfx::load_font("/usr/share/fonts/TTF/DejaVuSans.ttf");

  main_log.logf("Finished", font);


  if(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) != 0) {
    main_log.logf<Log::ERROR>("%s", SDL_GetError());
  }

  window = SDL_CreateWindow(
      "Scary Colors",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      window_size.x,
      window_size.y,
      SDL_WINDOW_OPENGL);

  if(window != nullptr) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

    if(gl_ctx != nullptr) {
      message_log.set_draw_rect(Rect2i(0, 0, window_size.x, 100));
      grid_world.set_draw_rect(Rect2i(Vec2i(0, 0), window_size - Vec2i(0, 100)));
      grid_world.set_tile_size(tile_size);

      gfx::load();

      game::view(&muh_view);
      game::load_world();

      run();

      gfx::unload();

      SDL_GL_DeleteContext(gl_ctx);
      gl_ctx = nullptr;
    } else {
      main_log.logf<Log::ERROR>("Failed to create GL context: %s\n", SDL_GetError());
    }

    SDL_DestroyWindow(window);
    window = nullptr;
  } else {
    main_log.logf<Log::ERROR>("Failed to create SDL window: %s\n", SDL_GetError());
  }


  TTF_Quit();
  SDL_Quit();

  return 0;
}

