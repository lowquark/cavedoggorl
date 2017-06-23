
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


#include <game/Engine.hpp>
#include <game/Game.hpp>
#include <game/FOV.hpp>
#include <gfx/gfx.hpp>
#include <gfx/draw.hpp>
#include <util/Vec2.hpp>
#include <util/Log.hpp>


Vec2u window_size;

SDL_Window * window = nullptr;
bool quit_signal = false;


gfx::GridWorld grid_world;
gfx::HUDOverlay hud;
gfx::WorldMessageLog message_log;


class MuhView : public game::View {
  private:
  void notify_tiles_update() override {
    grid_world.set_size(tiles().size());

    for(unsigned int y = 0 ; y < tiles().size().y ; y ++) {
      for(unsigned int x = 0 ; x < tiles().size().x ; x ++) {
        Vec2i pos((int)x, (int)y);
        grid_world.set_tile(pos, tiles().get(pos).glyph_id);
      }
    }
  }

  void notify_update_tile(Vec2i pos) override {
  }

  void notify_clear_entities() override {
    printf("%s\n", __PRETTY_FUNCTION__);
    grid_world.clear_sprites();
  }
  void notify_entity_update(unsigned int id) override {
    printf("%s\n", __PRETTY_FUNCTION__);
    auto kvpair_it = entities().find(id);

    if(kvpair_it != entities().end()) {
      auto & e = kvpair_it->second;
      gfx::Color gfx_color;
      gfx_color.r = 1.0f;
      gfx_color.g = 1.0f;
      gfx_color.b = 1.0f;
      grid_world.add_agent(id, e.glyph_id, e.pos, gfx_color);
    }
  }
  void notify_entity_remove(unsigned int id) override {
    printf("%s\n", __PRETTY_FUNCTION__);
    grid_world.remove_agent(id);
  }
  void notify_entity_move(unsigned int id, Vec2i from, Vec2i to) override {
    printf("%s\n", __PRETTY_FUNCTION__);
    grid_world.move_agent(id, from, to);
  }
};

Vec2i mouse_tile;

class MuhGameHooks : public game::Game::HookHandler {
  std::unique_ptr<game::Action> player_action() {
    printf("%s\n", __PRETTY_FUNCTION__);

    while(true) {
      SDL_Event event;
      while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
          return nullptr;
        }

        if(event.type == SDL_KEYDOWN) {
          if(event.key.keysym.sym == SDLK_0) {
            return nullptr;
          }

          grid_world.skip_animations();

          if(event.key.keysym.sym == SDLK_KP_6 || event.key.keysym.sym == SDLK_d) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(1, 0)));
          } else if(event.key.keysym.sym == SDLK_KP_9) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(1, -1)));
          } else if(event.key.keysym.sym == SDLK_KP_8 || event.key.keysym.sym == SDLK_w) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(0, -1)));
          } else if(event.key.keysym.sym == SDLK_KP_7) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(-1, -1)));
          } else if(event.key.keysym.sym == SDLK_KP_4 || event.key.keysym.sym == SDLK_a) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(-1, 0)));
          } else if(event.key.keysym.sym == SDLK_KP_1) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(-1, 1)));
          } else if(event.key.keysym.sym == SDLK_KP_2 || event.key.keysym.sym == SDLK_s) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(0, 1)));
          } else if(event.key.keysym.sym == SDLK_KP_3) {
            return std::unique_ptr<game::MoveAction>(new game::MoveAction(Vec2i(1, 1)));
          }

          if(event.key.keysym.sym == SDLK_COMMA || event.key.keysym.sym == SDLK_PERIOD) {
            if(event.key.keysym.mod & KMOD_SHIFT) {
              return std::unique_ptr<game::StairAction>(new game::StairAction);
            }
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

    return nullptr;
  }
};

MuhGameHooks muh_game_hooks;
MuhView muh_view;
world::SPCaveWorld muh_world;
game::Game muh_game(muh_game_hooks, muh_view, muh_world);


Log main_log;

int main(int argc, char ** argv) {
  // I hate these
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  const char * font = "Kingthings_Exeter.ttf";
  main_log.logf("Loading font from %s...", font);
  main_log.flush();
  gfx::load_font(font);
  main_log.logf("Finished", font);

  const Vec2u tile_size(16, 16);
  const Vec2u view_size(30, 30);
  window_size = Vec2u(tile_size.x * view_size.x, tile_size.y * view_size.y);

  gfx::load_tiles("./tiles.png", tile_size);
  gfx::draw::set_window_size(window_size);

  // create window
  if(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) != 0) {
    main_log.logf<Log::ERROR>("%s", SDL_GetError());
  }

  window = SDL_CreateWindow(
      "cavedogrl",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      window_size.x,
      window_size.y,
      SDL_WINDOW_OPENGL);

  // grafix config
  message_log.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
  grid_world.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
  grid_world.set_camera_size(view_size);
  grid_world.set_camera_margin(10);

  if(window != nullptr) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

    if(gl_ctx != nullptr) {
      glewInit(); 

      // grafix load to grafix card
      gfx::load();

      // start le engine

      muh_game.run();

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

