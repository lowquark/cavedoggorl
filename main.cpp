
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


static std::string original_message = "They're going to eat you!\nDon't just stand there! --RUN!!";

static draw::FontAtlas font_atlas;
static draw::TextBin muh_text(font_atlas);

static constexpr int TILES_X = 30;
static constexpr int TILES_Y = 24;
//static constexpr int WINDOW_WIDTH  = TILE_WIDTH * TILES_X;
//static constexpr int WINDOW_HEIGHT = TILE_HEIGHT * TILES_Y;

static const Vec2u window_size(TILE_WIDTH * TILES_X, TILE_HEIGHT * TILES_Y);

SDL_Window * window = nullptr;
bool quit_signal = false;


gfx::View muh_view;
int skip_countdown = 0;

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

      muh_view.skip_animations();

      if(muh_view.are_animations_finished()) {
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
        muh_view.skip_animations();
        skip_countdown = 2;
      }
    }
  }

  glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0f, window_size.x, window_size.y, 0.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_STENCIL_TEST);

  draw::clip(muh_view.rect());
  muh_view.update();
  draw::unclip();

  // Draw some cool text
  /*
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0f, window_size.x, window_size.y, 0.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  */

  /*
  if((rand() % 30) == 0) {
    std::string text = original_message;
    for(size_t i = 0 ; i < text.size() ; i ++) {
      if(text[i] != '\n' && text[i] != ' ') {
        text[i] = (rand() % 64) + 32;
      }
    }
    muh_text.set_text(text);
    muh_text.update_layout();
  } else {
    muh_text.set_text(original_message);
    muh_text.update_layout();
  }
  */

  Vec2i mouse_pos;
  SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

  Vec2i tile_pos = muh_view.mouse_location(mouse_pos - muh_view.rect().pos);
  Vec2i text_pos = mouse_pos + Vec2i(20, 0);

  std::string look_str;
  if(muh_view.look_str(look_str, tile_pos)) {
    muh_text.set_text(look_str);
    muh_text.update_layout();

    glTranslatef(text_pos.x, text_pos.y, 0.0f);

    draw::draw_rect(muh_text.rect(), Color(0.05f, 0.05f, 0.05f));
    draw::clip(muh_text.rect());
    muh_text.draw();
    draw::unclip();

    glTranslatef(-text_pos.x, -text_pos.y, 0.0f);
  }


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

int main(int argc, char ** argv) {
  // I hate these
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();


  const char * font = "/usr/share/fonts/TTF/DejaVuSans.ttf";
  printf("Loading font from %s... ", font);
  fflush(stdout);

  font_atlas.set_font(font);

  for(int i = 32 ; i < 128 ; i ++) {
    uint32_t code_point = i;
    font_atlas.load(code_point);
  }

  printf("Finished\n");
  fflush(stdout);

  if(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) != 0) {
    printf("%s\n", SDL_GetError());
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
      muh_view.set_rect(Rect2i(Vec2i(0, 100), window_size - Vec2i(0, 200)));

      gfx::load();
      font_atlas.load_textures();

      muh_text.set_rect(Rect2i(0, 0, 300, 20));
      muh_text.set_text(original_message);
      muh_text.update_layout();

      game::view(&muh_view);
      game::load_world();

      run();

      font_atlas.unload_textures();

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


  TTF_Quit();
  SDL_Quit();

  return 0;
}

