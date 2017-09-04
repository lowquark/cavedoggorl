
#include "gfx.hpp"

#include <rf/util/load_png.hpp>

#include <cmath>

namespace rf {
  namespace gfx {
    static draw::FontAtlas font_atlas;
    static draw::TileMapShader tilemap_shader;

    static Image tileset_img;
    static gl::Texture tileset;

    static Vec2u tileset_size;
    static Vec2u tile_size;

    float exponential_ease_in(float p) {
      return (p == 0.0) ? p : std::pow(2, 10 * (p - 1));
    }

    float exponential_ease_out(float p) {
      return (p == 1.0) ? p : 1 - std::pow(2, -10 * p);
    }

    // Modeled after the parabola y = x^2
    float quadratic_ease_in(float p) {
      return p * p;
    }

    // Modeled after the parabola y = -x^2 + 2x
    float quadratic_ease_out(float p) {
      return -(p * (p - 2));
    }

    void GridWorld::clear_sprites() {
      for(auto & a : queued_animations) {
        delete a;
      }
      queued_animations.clear();

      for(auto & a : active_animations) {
        delete a;
      }
      active_animations.clear();

      tile_sprites.clear();
      agent_sprites.clear();
    }

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
      auto sprite_kvpair_it = agent_sprites.find(followed_agent_id);
      if(sprite_kvpair_it != agent_sprites.end()) {
        auto & sprite = sprite_kvpair_it->second;
        auto pos = Vec2i(std::round(sprite.pos.x), std::round(sprite.pos.y));
        update_camera(pos, _camera_margin);
      }

      tilemap.set_tileset(tileset, tileset_size, tile_size);

      for(int j = 0 ; j < _camera_rect.size.y ; j ++) {
        for(int i = 0 ; i < _camera_rect.size.x ; i ++) {
          auto pos = Vec2i(i, j);
          auto tile = tile_sprites.get(_camera_rect.pos + pos);
          // HACK ALERT
          if(tile.visible || true) {
            if(tile.type_id == 1) {
              tilemap.set_tile(pos, 86);
              tilemap.set_fg_color(pos, 0x77, 0x66, 0x55);
              tilemap.set_bg_color(pos, 0x11, 0x11, 0x22);
            } else if(tile.type_id == 2) {
              tilemap.set_tile(pos, 133);
              tilemap.set_fg_color(pos, 0x77, 0x66, 0x55);
              tilemap.set_bg_color(pos, 0x11, 0x11, 0x22);
            } else {
              tilemap.set_tile(pos, 4);
              tilemap.set_fg_color(pos, 0x11, 0x11, 0x22);
              tilemap.set_bg_color(pos, 0x11, 0x11, 0x22);
            }
          } else if(tile.visited) {
            if(tile.type_id == 1) {
              tilemap.set_tile(pos, 86);
              tilemap.set_fg_color(pos, 0x44, 0x33, 0x33);
              tilemap.set_bg_color(pos, 0x11, 0x11, 0x22);
            } else if(tile.type_id == 2) {
              tilemap.set_tile(pos, 133);
              tilemap.set_fg_color(pos, 0x44, 0x33, 0x33);
              tilemap.set_bg_color(pos, 0x11, 0x11, 0x22);
            } else {
              tilemap.set_tile(pos, 4);
              tilemap.set_fg_color(pos, 0x11, 0x11, 0x22);
              tilemap.set_bg_color(pos, 0x11, 0x11, 0x22);
            }
          } else {
            tilemap.set_tile(pos, 4);
            tilemap.set_fg_color(pos, 0x11, 0x11, 0x22);
            tilemap.set_bg_color(pos, 0x11, 0x11, 0x22);
          }
        }
      }

      for(auto & kvpairs : agent_sprites) {
        auto & sprite = kvpairs.second;

        auto pos = Vec2i(std::round(sprite.pos.x), std::round(sprite.pos.y));
        auto screen_pos = pos - _camera_rect.pos;

        auto tile = tile_sprites.get(pos);

        // HACK ALERT
        if(tile.visible || true) {
          if(sprite.type_id == 0) {
            tilemap.set_tile(screen_pos, 3);
            tilemap.set_fg_color(screen_pos, 0xFF, 0xCC, 0x99);
          } else if(sprite.type_id == 1) {
            tilemap.set_tile(screen_pos, 0);
            tilemap.set_fg_color(screen_pos, 0xFF, 0xCC, 0x99);
          }
        }
      }

      //draw::clip(_draw_rect);
      tilemap_shader.draw(tilemap, _draw_rect.pos);
      //draw::unclip();
    }

    void GridWorld::update_camera(Vec2i focus, int margin) {
      // TODO: Handle case where camera is larger than world
      if(focus.x < _camera_rect.pos.x + margin) {
        _camera_rect.pos.x = focus.x - margin;
        //printf("_camera_rect.pos.x = focus.x - margin\n");
      }
      if(focus.y < _camera_rect.pos.y + margin) {
        _camera_rect.pos.y = focus.y - margin;
        //printf("_camera_rect.pos.y = focus.y - margin\n");
      }

      if(focus.x > _camera_rect.pos.x + _camera_rect.size.x - margin - 1) {
        _camera_rect.pos.x = focus.x + margin - _camera_rect.size.x + 1;
        //printf("_camera_rect.pos.x = focus.x + margin - _camera_rect.size.x\n");
      }
      if(focus.y + margin - _camera_rect.size.y > _camera_rect.pos.y - 1) {
        _camera_rect.pos.y = focus.y + margin - _camera_rect.size.y + 1;
        //printf("_camera_rect.pos.y = focus.y + margin - _camera_rect.size.y\n");
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

    void GridWorld::set_size(Vec2u size) {
      tile_sprites.resize(size);
      tilemap.set_size(size);
    }
    void GridWorld::set_tile(const Vec2i & pos, unsigned int type_id) {
      TileSprite sprite;
      sprite.type_id = type_id;
      tile_sprites.set(pos, sprite);
    }
    void GridWorld::clear_tile(const Vec2i & pos) {
      tile_sprites.set(pos, TileSprite());
    }
    void GridWorld::add_agent(unsigned int agent_id, unsigned int type_id, const Vec2i & pos, const Color & color) {
      AgentSprite sprite;
      sprite.type_id = type_id;
      sprite.pos = pos;
      sprite.color = color;
      agent_sprites[agent_id] = sprite;
    }
    void GridWorld::remove_agent(unsigned int agent_id) {
      agent_sprites.erase(agent_id);
    }

    Vec2i GridWorld::grid_pos(Vec2i screen_pos) const {
      return Vec2i(screen_pos.x / tile_size.x, screen_pos.y / tile_size.y) + _camera_rect.pos;
    }
    Vec2i GridWorld::screen_pos(Vec2i grid_pos) const {
      Vec2i local_grid_pos = grid_pos - _camera_rect.pos;
      return Vec2i(local_grid_pos.x * tile_size.x + tile_size.x / 2,
                   local_grid_pos.y * tile_size.y + tile_size.y / 2);
    }

    void GridWorld::follow_agent(unsigned int agent_id) {
      followed_agent_id = agent_id;
    }

    void GridWorld::set_fov(const rf::FOV & fov) {
      printf("%s\n", __PRETTY_FUNCTION__);

      for(int j = 0 ; j < tile_sprites.size().x ; j ++) {
        for(int i = 0 ; i < tile_sprites.size().y ; i ++) {
          rf::Vec2i pos(i, j);
          auto tile = tile_sprites.get(pos);
          if(fov.is_visible(pos)) {
            tile.visible = true;
            tile.visited = true;
          } else {
            tile.visible = false;
          }
          tile_sprites.set(pos, tile);
        }
      }
    }

    bool GridWorld::look_str(std::string & dst, Vec2i location) const {
      for(auto & kvpairs : agent_sprites) {
        auto & sprite = kvpairs.second;
        auto & id = kvpairs.first;

        Vec2i pos(round(sprite.pos.x), round(sprite.pos.y));
        if(pos == location) {
          if(id == 1) {
            dst = "Hark! Hark!";
            return true;
          } else {
            dst = "Orc";
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


    void HUDOverlay::look(Vec2i screen_pos, const std::string & look_str) {
      this->look_pos = screen_pos;
      this->look_str = look_str;

      text_bin.set(font_atlas, look_str);

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
        Vec2i text_offset = Vec2i(15, -text_bin.draw_rect().size.y/2 + offset);
        Vec2i bin_pos = look_pos + text_offset;

        Vec2i text_size = text_bin.text_size();
        Vec2i padding(4, 4);

        text_bin.set_draw_rect(Rect2i(bin_pos, Vec2i(clip_ease*text_size.x, text_size.y)));

        draw::draw_quad(Rect2i(bin_pos - padding, text_bin.draw_rect().size + padding*2), Color(0.05f, 0.05f, 0.05f));

        text_bin.draw();
      }
    }

    void WorldMessageLog::push(const std::string & message) {
      if(items.size() > 5) {
        items.pop_front();
      }
      Item item;
      item.text_bin.set(font_atlas, message);
      items.push_back(item);

      const int y_pad = 10;
      int y = 0;
      for(auto it = items.rbegin() ;
          it != items.rend() ;
          it ++) {
        auto & item = *it;

        item.pos = Vec2i(0, y);
        y += item.text_bin.text_size().y + y_pad;
      }
    }

    void WorldMessageLog::tick() {
    }
    void WorldMessageLog::draw() {
      draw::clip(_draw_rect);
      glTranslatef(_draw_rect.pos.x, _draw_rect.pos.y, 0.0f);

      for(unsigned int i = 0 ; i < items.size() ; i ++) {
        auto & item = items[i];
        auto text_size = item.text_bin.text_size();
        Vec2i bin_pos(0, _draw_rect.size.y - item.pos.y - text_size.y);
        Vec2i padding(4, 4);

        item.text_bin.set_draw_rect(Rect2i(bin_pos, text_size));

        draw::draw_quad(Rect2i(bin_pos - padding, text_size + padding*2), Color(0.05f, 0.05f, 0.05f));

        item.text_bin.draw();
      }

      glTranslatef(-_draw_rect.pos.x, -_draw_rect.pos.y, 0.0f);
      draw::unclip();
    }


    void load_font(const char * ttf_path) {
      font_atlas.set_font(ttf_path);

      for(int i = 32 ; i < 128 ; i ++) {
        uint32_t code_point = i;
        font_atlas.load(code_point);
      }
    }
    bool load_tiles(const char * png_path, Vec2u _tile_size) {
      if(_tile_size.x == 0 || _tile_size.y == 0) {
        return false;
      }

      if(load_png(tileset_img, "tiles.png")) {
        tile_size = _tile_size;

        tileset_size.x = tileset_img.width() / tile_size.x;
        tileset_size.y = tileset_img.height() / tile_size.y;

        return true;
      } else {
        return false;
      }
    }

    void load() {
      tilemap_shader.load("#version 130\n\n"
                           "in vec2 vertex_pos;\n"
                           "in vec2 vertex_texcoord;\n"
                           "varying vec2 texcoord;\n"
                           "void main() {\n"
                           "gl_Position.xy = vertex_pos;\n"
                           "gl_Position.z = 0.0;\n"
                           "gl_Position.w = 1.0;\n"
                           "texcoord = vertex_texcoord;\n"
                           "}",
                           "#version 130\n\n"
                           "uniform sampler2D tileset;\n"
                           "uniform sampler2D fg_color;\n"
                           "uniform sampler2D bg_color;\n"
                           "uniform sampler2D index_data;\n"
                           "uniform ivec2 tilemap_size;\n"
                           "uniform ivec2 tileset_size;\n"
                           "\n"
                           "varying vec2 texcoord;\n"
                           "void main() { \n"
                           "vec4 fg = texture(fg_color, texcoord);\n"
                           "vec4 bg = texture(bg_color, texcoord);\n"
                           "vec2 tileset_coord = texture(index_data, texcoord).xy * 255/256;\n"
                           "\n"
                           "vec2 tile_local_texcoord = texcoord*tilemap_size - floor(texcoord*tilemap_size);\n"
                           "vec2 tileset_texcoord = tileset_coord + tile_local_texcoord/tileset_size;\n"
                           "vec4 tile_color = texture(tileset, tileset_texcoord);\n"
                           "\n"
                           "if(abs(tile_color.r - tile_color.g) < 0.001 && \n"
                           "   abs(tile_color.g - tile_color.b) < 0.001) {\n"
                           "gl_FragColor = bg + tile_color.r*(fg - bg);\n"
                           "} else {\n"
                           "gl_FragColor = tile_color;\n"
                           "}\n"
                           "}\n"); // lol

      tileset.load(tileset_img);
      printf("tileset.id(): %d\n", tileset.id());

      font_atlas.load_textures();
    }
    void unload() {
      tilemap_shader.unload();

      tileset.unload();
      tileset_size = Vec2u(0, 0);
      tile_size = Vec2u(0, 0);

      font_atlas.unload_textures();
    }
  }
}

