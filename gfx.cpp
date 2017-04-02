
#include "gfx.hpp"

namespace gfx {
  static draw::FontAtlas font_atlas;
  static draw::TextBin muh_text(font_atlas);

  static std::string original_message = "They're going to eat you!\nDon't just stand there! --RUN!!";

  class AgentMoveAnimation : public Animation {
    AgentSprite & sprite;
    Vec2i from;
    Vec2i to;

    unsigned int t_animation;

    public:
    AgentMoveAnimation(AgentSprite & sprite, const Vec2i & from, const Vec2i & to, unsigned int t_animation)
      : sprite(sprite)
      , from(from)
      , to(to)
      , t_animation(t_animation) {
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


  void View::on_world_load(unsigned int tiles_x, unsigned int tiles_y) {
    printf("%s: %ux%u\n", __PRETTY_FUNCTION__, tiles_x, tiles_y);
    tile_sprites.resize(tiles_x, tiles_y);
  }
  void View::on_tile_load(game::Id type_id, const Vec2i & pos) {
    TileSprite sprite;
    sprite.type_id = type_id;
    tile_sprites.set(pos, sprite);
  }
  void View::on_agent_load(game::Id agent_id, game::Id type_id, const Vec2i & pos, const Color & color) {
    AgentSprite sprite;
    sprite.type_id = type_id;
    sprite.pos = pos;
    sprite.color = color;
    agent_sprites[agent_id] = sprite;
  }

  void View::on_agent_move(game::Id agent_id, const Vec2i & from, const Vec2i & to) {
    auto agent_kvpair_it = agent_sprites.find(agent_id);

    if(agent_kvpair_it != agent_sprites.end()) {
      auto & agent = agent_kvpair_it->second;
      queued_animations.push_back(new AgentMoveAnimation(agent, from, to, 3));
    }
  }
  void View::on_agent_death(game::Id agent_id) {
  }

  void View::step_animations() {
    if(active_animations.empty()) {
      if(!queued_animations.empty()) {
        auto anim = queued_animations.front();
        queued_animations.pop_front();
        active_animations.push_back(anim);
        active_animations.back()->start();
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

  void View::draw() {
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, _window_size.x, _window_size.y, 0.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto sprite_kvpair_it = agent_sprites.find(1);
    if(sprite_kvpair_it != agent_sprites.end()) {
      auto & sprite = sprite_kvpair_it->second;
      draw::camera_pos = sprite.pos - Vec2f(_window_size.x / TILE_WIDTH /2, _window_size.y / TILE_HEIGHT / 2);
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

      /*
      // Draw the path for debugging
      auto agent = game::debug::get_agent(kvpairs.first);
      if(agent) {
        draw::draw_path(agent->path, agent->color);
      }
      */
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, _window_size.x, _window_size.y, 0.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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

    muh_text.draw();
  }

  void View::skip_animations() {
    for(auto & anim : active_animations) {
      // Also finish active animations
      anim->finish();
      delete anim;
    }

    active_animations.clear();
    // Purge everything. EVERYTHING.
    for(auto & anim : queued_animations) {
      // These animations haven't been started yet
      anim->start();
      anim->finish();
      delete anim;
    }
    queued_animations.clear();
  }
  bool View::are_animations_finished() {
    return queued_animations.empty() && active_animations.empty();
  }

  void View::update() {
    step_animations();
    draw();
  }


  void init() {
    // I hate these
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
  }
  void deinit() {
    TTF_Quit();
  }

  void load() {
    font_atlas.load_textures();

    muh_text.set_rect(Rect2i(50, 50, 100, 100));
    muh_text.set_text(original_message);
    muh_text.update_layout();
  }
  void unload() {
    font_atlas.unload_textures();
  }
}

