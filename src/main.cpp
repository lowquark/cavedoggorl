
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


#include <rf/gfx/gfx.hpp>
#include <rf/gfx/draw.hpp>
#include <rf/util/Vec2.hpp>
#include <rf/util/Log.hpp>
#include <rf/game/Game.hpp>


using namespace rf;

Vec2u window_size;

SDL_Window * window = nullptr;
bool quit_signal = false;

gfx::GridWorld grid_world;
gfx::HUDOverlay hud;
gfx::WorldMessageLog message_log;
Vec2i mouse_tile;

GameSave super_save;
Game super_game(super_save);

bool stop = false;
void handle_turn() {
  // wait for input
  SDL_Event event;

  while(true) {
    SDL_WaitEvent(&event);

    if(event.type == SDL_QUIT) {
      stop = true;
      return;
    } else if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_0) {
        stop = true;
        return;
      } else {
        // apply input
        if(event.key.keysym.sym == SDLK_KP_5 || event.key.keysym.sym == SDLK_w) {
          super_game.step(PlayerWaitAction());
        } else if(event.key.keysym.sym == SDLK_KP_6 || event.key.keysym.sym == SDLK_d) {
          super_game.step(PlayerMoveAction(Vec2i(1, 0)));
        } else if(event.key.keysym.sym == SDLK_KP_9) {
          super_game.step(PlayerMoveAction(Vec2i(1, -1)));
        } else if(event.key.keysym.sym == SDLK_KP_8 || event.key.keysym.sym == SDLK_w) {
          super_game.step(PlayerMoveAction(Vec2i(0, -1)));
        } else if(event.key.keysym.sym == SDLK_KP_7) {
          super_game.step(PlayerMoveAction(Vec2i(-1, -1)));
        } else if(event.key.keysym.sym == SDLK_KP_4 || event.key.keysym.sym == SDLK_a) {
          super_game.step(PlayerMoveAction(Vec2i(-1, 0)));
        } else if(event.key.keysym.sym == SDLK_KP_1) {
          super_game.step(PlayerMoveAction(Vec2i(-1, 1)));
        } else if(event.key.keysym.sym == SDLK_KP_2 || event.key.keysym.sym == SDLK_s) {
          super_game.step(PlayerMoveAction(Vec2i(0, 1)));
        } else if(event.key.keysym.sym == SDLK_KP_3) {
          super_game.step(PlayerMoveAction(Vec2i(1, 1)));
        }
        return;
      }
    }


    Vec2i mouse_pos;
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

    Vec2i new_mouse_tile = grid_world.grid_pos(mouse_pos - grid_world.draw_rect().pos);
    if(new_mouse_tile != mouse_tile) {
      mouse_tile = new_mouse_tile;

      std::string look_str;
      if(grid_world.look_str(look_str, mouse_tile)) {
        hud.look(grid_world.draw_rect().pos + grid_world.screen_pos(mouse_tile), look_str);
      } else {
        hud.look_finish();
      }
    }


    grid_world.tick();
    hud.tick();
    message_log.tick();


    glClearColor(0.1f, 0.0f, 0.1f, 0.0f);
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
}

class SuperVisitor : public GameEventVisitor {
  void visit(const MissileEvent & event) override {
  }
};

void run() {
  while(!stop) {
    // draw world

    if(super_game.is_player_turn()) {
      handle_turn();
    } else {
      super_game.step();
    }

    SuperVisitor v;
    super_game.handle_events(v);

    // draw animations
    //...
  }

  super_game.save();
}

int main(int argc, char ** argv) {
  // I hate these
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  warn("hi");
  log("hi");
  warntopic("topic!", "hi");
  logtopic("topic!", "hi");
  warnf("hi %d", 5);
  logf("hi %d", 5);
  warntopicf("topic!", "hi %d", 5);
  logtopicf("topic!", "hi %d", 5);

  const char * font = "Kingthings_Exeter.ttf";
  printf("Loading font from %s...\n", font);
  fflush(stdout);
  gfx::load_font(font);
  printf("Finished (%p)\n", font);

  const Vec2u tile_size(16, 16);
  const Vec2u view_size(30, 30);
  window_size = Vec2u(tile_size.x * view_size.x, tile_size.y * view_size.y);

  gfx::load_tiles("./tiles.png", tile_size);
  gfx::draw::set_window_size(window_size);

  // create window
  if(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) != 0) {
    printf("%s\n", SDL_GetError());
  }

  window = SDL_CreateWindow(
      "cavedoggorl",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      window_size.x,
      window_size.y,
      SDL_WINDOW_OPENGL);

  // grafix config
  message_log.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
  grid_world.set_size(Vec2u(32, 32));
  grid_world.set_tile(Vec2i(2, 2), 1);
  grid_world.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
  grid_world.set_camera_size(view_size);
  grid_world.set_camera_margin(10);

  if(window != nullptr) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

    if(gl_ctx != nullptr) {
      glewInit(); 

      // grafix load to grafix card
      gfx::load();

      // start le game
      run();

      gfx::unload();

      SDL_GL_DeleteContext(gl_ctx);
      gl_ctx = nullptr;
    } else {
      printf("Failed to create GL context: %s\n", SDL_GetError());
    }

    SDL_DestroyWindow(window);
    window = nullptr;
  } else {
    printf("Failed to create SDL window: %s\n", SDL_GetError());
  }


  TTF_Quit();
  SDL_Quit();

  return 0;
}

