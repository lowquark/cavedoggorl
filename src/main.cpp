
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


#include <game/game.hpp>
#include <game/FOV.hpp>
#include <gfx/gfx.hpp>
#include <util/Vec2.hpp>
#include <util/Log.hpp>


static constexpr int TILES_X = 30;
static constexpr int TILES_Y = 24;

static const Vec2u tile_size(32, 32);
static const Vec2u window_size(tile_size.x * TILES_X, tile_size.y * TILES_Y);


SDL_Window * window = nullptr;
bool quit_signal = false;


game::World muh_world;

gfx::GridWorld grid_world;
gfx::HUDOverlay hud;
gfx::WorldMessageLog message_log;


class MuhView : public game::View {
  void set_world_size(unsigned int tiles_x, unsigned int tiles_y) override {
    grid_world.set_size(Vec2u(tiles_x, tiles_y));
  }
  void set_tile(const Vec2i & pos, unsigned int type_id) override {
    grid_world.set_tile(pos, type_id);
  }
  void clear_tile(const Vec2i & pos) override {
    grid_world.clear_tile(pos);
  }

  void set_fov(const FOV & fov) override {
    printf("%s\n", __PRETTY_FUNCTION__);
  }

  void set_glyph(game::ObjectHandle obj,
                 unsigned int type_id,
                 const Vec2i & pos,
                 const game::Color & color) override {
    gfx::Color gfx_color;
    gfx_color.r = (float) color.r / 255;
    gfx_color.g = (float) color.g / 255;
    gfx_color.b = (float) color.b / 255;
    grid_world.add_agent(obj.id(), type_id, pos, gfx_color);
  }
  void clear_glyph(game::ObjectHandle obj) override {
    grid_world.remove_agent(obj.id());
  }
  void move_glyph(game::ObjectHandle obj,
                  const Vec2i & from,
                  const Vec2i & to) override {
    grid_world.move_agent(obj.id(), from, to);
  }

  void message(const std::string & message) override {
    message_log.push(message);
  }

  void follow(game::ObjectHandle obj) override {
    grid_world.follow_agent(obj.id());
  }

  void clear() override {
    grid_world.clear_sprites();
  }
};

MuhView muh_view;


Vec2i mouse_tile;

bool is_player_turn = false;
void step_game() {
  printf("step_game()\n");
  is_player_turn = muh_world.tick_until_player_turn(200).first;
}

void update() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    if(event.type == SDL_QUIT) {
      quit_signal = true;
      return;
    }

    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_0) {
        quit_signal = true;
        return;
      }
      if(is_player_turn) {
        grid_world.skip_animations();

        if(event.key.keysym.sym == SDLK_KP_6 || event.key.keysym.sym == SDLK_d) {
          muh_world.player_move_attack(Vec2i( 1,  0));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_9) {
          muh_world.player_move_attack(Vec2i( 1, -1));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_8 || event.key.keysym.sym == SDLK_w) {
          muh_world.player_move_attack(Vec2i( 0, -1));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_7) {
          muh_world.player_move_attack(Vec2i(-1, -1));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_4 || event.key.keysym.sym == SDLK_a) {
          muh_world.player_move_attack(Vec2i(-1,  0));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_1) {
          muh_world.player_move_attack(Vec2i(-1,  1));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_2 || event.key.keysym.sym == SDLK_s) {
          muh_world.player_move_attack(Vec2i( 0,  1));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_3) {
          muh_world.player_move_attack(Vec2i( 1,  1));
          step_game();
        }

        /*
        if(event.key.keysym.sym == SDLK_COMMA) {
          if(event.key.keysym.mod & KMOD_SHIFT) {
            muh_world.player_activate_tile();
            step_game();
          }
        }
        */
      }
    }
  }

  /*
  if(is_player_turn == false && grid_world.are_animations_finished()) {
    // should prompt for continue in this case
    step_game();
  }
  */

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

void run() {
  quit_signal = false;
  while(true) {
    update();
    if(quit_signal) { break; }
  }
}

Log main_log;

int main(int argc, char ** argv) {
  int x0 = 0;
  int y0 = 0;
  int x1 = 2;
  int y1 = 1;

  int dx = x1 - x0;
  int dy = y1 - y0;
  int y = y0, e = 0;

  for(int x = x0 ; x <= x1 ; x ++) {
    printf("x, y: %d, %d\n", x, y);
    e += dy;
    if(2*e >= dx) {
      y ++;
      e -= dx;
    }
  }

  Vec2i origin(10, 10);
  BresenhamFOV fov;
  Map<unsigned int> solid(21, 21);

  solid.set(Vec2i( 6,  5), 1);
  solid.set(Vec2i( 5,  6), 1);
  solid.set(Vec2i(11, 10), 1);
  solid.set(Vec2i( 9, 10), 1);


  for(int r = 5 ; r < 10 ; r ++) {
    fov.update(solid, origin, r);

    printf("r: %d\n", r);
    for(int j = 0 ; j < 21 ; j ++) {
      for(int i = 0 ; i < 21 ; i ++) {
        Vec2i p(i, j);
        if(fov.is_visible(p)) {
          if(p == origin) {
            printf("@");
          } else if(solid.get(p) == 1) {
            printf("#");
          } else {
            printf(".");
          }
        } else {
          printf(" ");
        }
      }
      printf("\n");
    }
    printf("\n");
  }


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
      message_log.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
      grid_world.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
      grid_world.set_tile_size(tile_size);

      gfx::load();

      //muh_world.add_view(&muh_view);

      auto hero_obj = muh_world.create_hero(Vec2i(1, 1));

      muh_world.set_size(30, 30);
      for(unsigned int j = 0 ; j < 30 ; j ++) {
        for(unsigned int i = 0 ; i < 30 ; i ++) {
          if(rand() % 5 == 0) {
            muh_world.set_tile(Vec2i(i, j), 1);
          } else {
            muh_world.set_tile(Vec2i(i, j), 2);
          }
        }
      }

      muh_world.create_badguy(Vec2i(2, 2), hero_obj);

      step_game();

      muh_world.add_view(&muh_view, hero_obj);

      //muh_game.create_new();
      //muh_game.save("asdf");
      //muh_game.load_old("asdf");

      run();

      //muh_game.save("asdf");

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

