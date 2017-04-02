
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


static constexpr int TILES_X = 30;
static constexpr int TILES_Y = 24;
static constexpr int WINDOW_WIDTH  = TILE_WIDTH * TILES_X;
static constexpr int WINDOW_HEIGHT = TILE_HEIGHT * TILES_Y;

SDL_Window * window = nullptr;
bool quit_signal = false;


gfx::View muh_view;
int skip_countdown = 0;

void update() {
  muh_view.update();
  SDL_GL_SwapWindow(window);

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

      if(muh_view.are_animations_finished() && skip_countdown == 0) {
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

  gfx::init();

  muh_view.window_size(Vec2u(WINDOW_WIDTH, WINDOW_HEIGHT));


  window = SDL_CreateWindow(
      "Scary Colors",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      WINDOW_WIDTH,
      WINDOW_HEIGHT,
      SDL_WINDOW_OPENGL);

  if(window != nullptr) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

    if(gl_ctx != nullptr) {
      gfx::load();

      game::view(&muh_view);
      game::load_world();

      run();

      gfx::unload();
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


  gfx::deinit();


  SDL_Quit();

  return 0;
}

