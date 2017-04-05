
#include "gfx.hpp"

#include <cmath>

namespace gfx {
  static draw::FontAtlas font_atlas;

  float exponential_ease_in(float p) {
      return (p == 0.0) ? p : std::pow(2, 10 * (p - 1));
  }

  float exponential_ease_out(float p) {
    return (p == 1.0) ? p : 1 - std::pow(2, -10 * p);
  }

  // Modeled after the parabola y = x^2
  float quadratic_ease_in(float p)
  {
    return p * p;
  }

  // Modeled after the parabola y = -x^2 + 2x
  float quadratic_ease_out(float p)
  {
    return -(p * (p - 2));
  }

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


  void GridWorld::tick() {
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

  void GridWorld::draw() {
    draw::clip(_draw_rect);
    draw::set_tile_size(_tile_size);

    glTranslatef(_draw_rect.pos.x, _draw_rect.pos.y, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO: Implement on_control_agent event to receive this id
    auto sprite_kvpair_it = agent_sprites.find(1);
    if(sprite_kvpair_it != agent_sprites.end()) {
      auto & sprite = sprite_kvpair_it->second;
      _camera_pos = sprite.pos - Vec2f((float)_draw_rect.size.x / _tile_size.x / 2,
                                       (float)_draw_rect.size.y / _tile_size.y / 2);
    }

    draw::set_camera_pos(_camera_pos);

    // TODO: Only iterate over visible tiles
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

      // TODO: Draw the agent only if not hidden
      draw::draw_agent(sprite.pos, sprite.type_id, sprite.color);

      /*
      // Draw the path for debugging purposes
      auto agent = game::debug::get_agent(kvpairs.first);
      if(agent) {
        draw::draw_path(agent->path, agent->color);
      }
      */
    }

    glTranslatef(-_draw_rect.pos.x, -_draw_rect.pos.y, 0.0f);
    draw::unclip();
  }

  void GridWorld::set_size(Vec2u size) {
    tile_sprites.resize(size.x, size.y);
  }
  void GridWorld::set_tile(const Vec2i & pos, game::Id type_id) {
    TileSprite sprite;
    sprite.type_id = type_id;
    tile_sprites.set(pos, sprite);
  }
  void GridWorld::add_agent(game::Id agent_id, game::Id type_id, const Vec2i & pos, const Color & color) {
    AgentSprite sprite;
    sprite.type_id = type_id;
    sprite.pos = pos;
    sprite.color = color;
    agent_sprites[agent_id] = sprite;
  }
  void GridWorld::remove_agent(game::Id agent_id) {
    agent_sprites.erase(agent_id);
  }

  void GridWorld::move_agent(game::Id agent_id, const Vec2i & from, const Vec2i & to) {
    auto agent_kvpair_it = agent_sprites.find(agent_id);

    if(agent_kvpair_it != agent_sprites.end()) {
      auto & agent = agent_kvpair_it->second;
      queued_animations.push_back(new AgentMoveAnimation(agent, from, to, 3));
    }
  }

  Vec2i GridWorld::grid_pos(Vec2i screen_pos) {
    return Vec2i(std::round((float)screen_pos.x / _tile_size.x) + _camera_pos.x,
                 std::round((float)screen_pos.y / _tile_size.y) + _camera_pos.y);
  }
  Vec2i GridWorld::screen_pos(Vec2i grid_pos) {
    return Vec2i((grid_pos.x - _camera_pos.x) * _tile_size.x,
                 (grid_pos.y - _camera_pos.y) * _tile_size.y);
  }

  bool GridWorld::look_str(std::string & dst, Vec2i location) const {
    for(auto & kvpairs : agent_sprites) {
      auto & sprite = kvpairs.second;
      auto & id = kvpairs.first;

      Vec2i pos(round(sprite.pos.x), round(sprite.pos.y));
      if(pos == location) {
        if(id == 1) {
          dst = "your nicely colored self";
          return true;
        } else {
          dst = "an evil looking color";
          return true;
        }
      }
    }
    return false;
  }

  void GridWorld::skip_animations() {
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
  bool GridWorld::are_animations_finished() {
    return queued_animations.empty() && active_animations.empty();
  }


  HUDOverlay::HUDOverlay() 
    : text_bin(font_atlas) {}

  void HUDOverlay::look(Vec2i screen_pos, const std::string & look_str) {
    this->look_pos = screen_pos;
    this->look_str = look_str;

    text_bin.set_text(look_str);
    text_bin.update_layout();

    look_enabled = true;
    look_ease_timer = look_ease_timer_max;
  }
  void HUDOverlay::look_finish() {
    look_enabled = false;
    look_ease_timer = 0;
  }

  void HUDOverlay::tick() {
    if(look_ease_timer > 0) {
      look_ease_timer --;
    }
  }
  void HUDOverlay::draw() {
    if(look_enabled) {
      const int dist = 20;

      float pos_ease = exponential_ease_out((float)(look_ease_timer_max - look_ease_timer) / look_ease_timer_max);
      float clip_ease = quadratic_ease_out((float)(look_ease_timer_max - look_ease_timer) / look_ease_timer_max);
      int offset = round(dist * (1.0f - pos_ease));

      // offset from center of tile to corner of text box
      Vec2i text_offset = Vec2i(15, -text_bin.rect().size.y/2 + offset);
      Vec2i bin_pos = look_pos + text_offset;

      glTranslatef(bin_pos.x, bin_pos.y, 0.0f);

      text_bin.set_rect(Rect2i(0, 0, clip_ease*300, 20));

      draw::draw_rect(text_bin.rect(), Color(0.05f, 0.05f, 0.05f));
      draw::clip(text_bin.rect());
      text_bin.draw();
      draw::unclip();

      glTranslatef(-bin_pos.x, -bin_pos.y, 0.0f);
    }
  }

  void load_font(const char * ttf_path) {
    font_atlas.set_font(ttf_path);

    for(int i = 32 ; i < 128 ; i ++) {
      uint32_t code_point = i;
      font_atlas.load(code_point);
    }
  }

  void load() {
    font_atlas.load_textures();
  }
  void unload() {
    font_atlas.unload_textures();
  }
}

