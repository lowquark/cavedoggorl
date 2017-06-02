
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
  unsigned int follow_obj_id = 0;

  public:
  void set_world_size(unsigned int tiles_x, unsigned int tiles_y) override {
    grid_world.set_size(Vec2u(tiles_x, tiles_y));
  }
  void set_tile(const Vec2i & pos, unsigned int type_id) override {
    grid_world.set_tile(pos, type_id);
  }
  void clear_tile(const Vec2i & pos) override {
    grid_world.clear_tile(pos);
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
    follow_obj_id = obj.id();
    grid_world.follow_agent(obj.id());
  }

  void set_fov(unsigned int id, const FOV & fov) override {
    printf("%s: id = %u\n", __PRETTY_FUNCTION__, id);

    if(id == follow_obj_id) {
      grid_world.set_fov(fov);
    }
  }

  void clear() override {
    grid_world.clear_sprites();
  }
};

MuhView muh_view;

game::Engine muh_engine(muh_view);


Vec2i mouse_tile;

bool is_player_turn = false;
void step_game() {
  printf("step_game()\n");
  is_player_turn = muh_engine.step(200).first;
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
          muh_engine.complete_turn(game::MoveAction(Vec2i( 1,  0)));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_9) {
          muh_engine.complete_turn(game::MoveAction(Vec2i( 1, -1)));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_8 || event.key.keysym.sym == SDLK_w) {
          muh_engine.complete_turn(game::MoveAction(Vec2i( 0, -1)));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_7) {
          muh_engine.complete_turn(game::MoveAction(Vec2i(-1, -1)));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_4 || event.key.keysym.sym == SDLK_a) {
          muh_engine.complete_turn(game::MoveAction(Vec2i(-1,  0)));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_1) {
          muh_engine.complete_turn(game::MoveAction(Vec2i(-1,  1)));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_2 || event.key.keysym.sym == SDLK_s) {
          muh_engine.complete_turn(game::MoveAction(Vec2i( 0,  1)));
          step_game();
        } else if(event.key.keysym.sym == SDLK_KP_3) {
          muh_engine.complete_turn(game::MoveAction(Vec2i( 1,  1)));
          step_game();
        }

        /*
        if(event.key.keysym.sym == SDLK_COMMA) {
          if(event.key.keysym.mod & KMOD_SHIFT) {
            muh_engine.player_activate_tile();
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
  // I hate these
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  //const char * font = "/usr/share/fonts/TTF/DejaVuSans.ttf";
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

  if(window != nullptr) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);

    if(gl_ctx != nullptr) {
      glewInit(); 

      message_log.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
      grid_world.set_draw_rect(Rect2i(Vec2i(0, 0), window_size));
      grid_world.set_camera_size(view_size);
      grid_world.set_camera_margin(10);

      gfx::load();

      unsigned int sid = muh_engine.create_space(Vec2u(60, 60));

      auto hero_obj = muh_engine.create_hero();
      muh_view.follow(hero_obj.id());
      muh_engine.spawn(hero_obj, sid, Vec2i(2, 1));

      for(int i = 0 ; i < 4 ; i ++) {
        muh_engine.spawn(muh_engine.create_badguy(hero_obj), sid, Vec2i(rand() % 60, rand() % 60));
      }

      step_game();

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

