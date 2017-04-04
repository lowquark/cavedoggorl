
#include "gfx.hpp"

#include <cmath>

namespace gfx {
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
    glTranslatef(_rect.pos.x, _rect.pos.y, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO: Implement on_control_agent event to receive this id
    auto sprite_kvpair_it = agent_sprites.find(1);
    if(sprite_kvpair_it != agent_sprites.end()) {
      auto & sprite = sprite_kvpair_it->second;
      draw::camera_pos = sprite.pos - Vec2f((float)_rect.size.x / TILE_WIDTH / 2,
                                            (float)_rect.size.y / TILE_HEIGHT / 2);
    }

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
      draw::draw_agent(sprite.pos, sprite.color);

      /*
      // Draw the path for debugging purposes
      auto agent = game::debug::get_agent(kvpairs.first);
      if(agent) {
        draw::draw_path(agent->path, agent->color);
      }
      */
    }

    glTranslatef(-_rect.pos.x, -_rect.pos.y, 0.0f);
  }

  Vec2i View::mouse_location(Vec2i local_mouse) {
    return Vec2i(std::round((float)local_mouse.x / TILE_WIDTH) + draw::camera_pos.x,
                 std::round((float)local_mouse.y / TILE_HEIGHT) + draw::camera_pos.y);
  }

  bool View::look_str(std::string & dst, Vec2i location) const {
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


  void load() {
  }
  void unload() {
  }
}

