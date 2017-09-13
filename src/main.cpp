
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <rf/gfx/gfx.hpp>
#include <rf/gfx/Scene.hpp>
#include <rf/gfx/draw.hpp>
#include <rf/util/Vec2.hpp>
#include <rf/util/Log.hpp>
#include <rf/util/Dijkstra.hpp>
#include <rf/game/Game.hpp>

using namespace rf;

Vec2u window_size;

SDL_Window * window = nullptr;
bool quit_signal = false;

gfx::Scene gfx_scene;
gfx::HUDOverlay hud;
gfx::WorldMessageLog message_log;
Vec2i mouse_tile;

game::GameSave super_save;
game::Game super_game(super_save);

static LogTopic & gfx_topic = logtopic("gfx");

bool stop = false;
void handle_turn() {
  // wait for input
  SDL_Event event;

  while(true) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
        stop = true;
        return;
      } else if(event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_0) {
          stop = true;
          return;
        } else {
          // apply input
          if(event.key.keysym.sym == SDLK_KP_5) {
            super_game.wait();
          } else if(event.key.keysym.sym == SDLK_KP_6 || event.key.keysym.sym == SDLK_d) {
            super_game.move(Vec2i(1, 0));
          } else if(event.key.keysym.sym == SDLK_KP_9) {
            super_game.move(Vec2i(1, -1));
          } else if(event.key.keysym.sym == SDLK_KP_8 || event.key.keysym.sym == SDLK_w) {
            super_game.move(Vec2i(0, -1));
          } else if(event.key.keysym.sym == SDLK_KP_7) {
            super_game.move(Vec2i(-1, -1));
          } else if(event.key.keysym.sym == SDLK_KP_4 || event.key.keysym.sym == SDLK_a) {
            super_game.move(Vec2i(-1, 0));
          } else if(event.key.keysym.sym == SDLK_KP_1) {
            super_game.move(Vec2i(-1, 1));
          } else if(event.key.keysym.sym == SDLK_KP_2 || event.key.keysym.sym == SDLK_s) {
            super_game.move(Vec2i(0, 1));
          } else if(event.key.keysym.sym == SDLK_KP_3) {
            super_game.move(Vec2i(1, 1));
          }
          return;
        }
      }
    }

    /*
    Vec2i mouse_pos;
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

    Vec2i new_mouse_tile = gfx_scene.grid_pos(mouse_pos - gfx_scene.draw_rect().pos);
    if(new_mouse_tile != mouse_tile) {
      mouse_tile = new_mouse_tile;

      std::string look_str;
      if(gfx_scene.look_str(look_str, mouse_tile)) {
        hud.look(gfx_scene.draw_rect().pos + gfx_scene.screen_pos(mouse_tile), look_str);
      } else {
        hud.look_finish();
      }
    }
    */

    gfx_scene.tick();
    hud.tick();
    message_log.tick();

    glClearColor((float)0x11/0xFF, (float)0x11/0xFF, (float)0x11/0xFF, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, window_size.x, window_size.y, 0.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_STENCIL_TEST);

    gfx_scene.set_state(super_game.draw(gfx_scene.viewport()));
    gfx_scene.draw();
    hud.draw();
    message_log.draw();

    SDL_GL_SwapWindow(window);

    SDL_Delay(14);
  }
}

/*
void GridWorld::update_camera(Vec2i focus, int margin) {
  // TODO: Handle case where camera is larger than world
  if(focus.x < _camera_rect.pos.x + margin) {
    _camera_rect.pos.x = focus.x - margin;
  }
  if(focus.y < _camera_rect.pos.y + margin) {
    _camera_rect.pos.y = focus.y - margin;
  }

  if(focus.x > _camera_rect.pos.x + _camera_rect.size.x - margin - 1) {
    _camera_rect.pos.x = focus.x + margin - _camera_rect.size.x + 1;
  }
  if(focus.y + margin - _camera_rect.size.y > _camera_rect.pos.y - 1) {
    _camera_rect.pos.y = focus.y + margin - _camera_rect.size.y + 1;
  }

  if(_camera_rect.pos.x < 0) {
    _camera_rect.pos.x = 0;
  }
  if(_camera_rect.pos.y < 0) {
    _camera_rect.pos.y = 0;
  }
  if(_camera_rect.pos.x + _camera_rect.size.x > tile_sprites.size().x) {
    _camera_rect.pos.x = tile_sprites.size().x - _camera_rect.size.x;
  }
  if(_camera_rect.pos.y + _camera_rect.size.y > tile_sprites.size().y) {
    _camera_rect.pos.y = tile_sprites.size().y - _camera_rect.size.y;
  }
}
*/

class SuperVisitor : public game::GameEventVisitor {
  void visit(const game::MissileEvent & event) override {
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
  DijkstraMap::test();

  // I hate these
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  logf("sizeof(rf::game::Object): %lu", sizeof(rf::game::Object));

  const Vec2u tile_size(16, 16);
  const Vec2u view_size(32, 32);
  window_size = Vec2u(tile_size.x * view_size.x, tile_size.y * view_size.y);

  gfx::draw::set_window_size(window_size);

  // create window
  if(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) != 0) {
    gfx_topic.warn(SDL_GetError());
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
  gfx_scene.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
  gfx_scene.set_viewport(Rect2i(0, 0, 30, 30));
  gfx_scene.set_tileset("file://tiles.png");
  gfx_scene.set_state(super_game.draw(gfx_scene.viewport()));

  if(window != nullptr) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

    if(gl_ctx != nullptr) {
      glewInit(); 

      // grafix load to grafix card
      gfx::load();
      gfx::load_font("./res/Kingthings_Exeter.ttf");
      gfx::load_tiles("./res/tiles.png");

      // start le game
      run();

      gfx::unload();

      SDL_GL_DeleteContext(gl_ctx);
      gl_ctx = nullptr;
    } else {
      gfx_topic.warnf("Failed to create GL context: %s", SDL_GetError());
    }

    SDL_DestroyWindow(window);
    window = nullptr;
  } else {
    gfx_topic.warnf("Failed to create SDL window: %s", SDL_GetError());
  }


  TTF_Quit();
  SDL_Quit();

  return 0;
}

