
#include "gfx.hpp"

#include <rf/util/load_png.hpp>
#include <rf/util/Log.hpp>

#include <cmath>
#include <memory>

namespace rf {
  namespace gfx {
    static std::shared_ptr<gl::Texture> tileset;

    static std::shared_ptr<draw::FontAtlas> font_atlas;
    std::unique_ptr<TilemapShader> tilemap_shader;

    static LogTopic & gfx_topic = logtopic("gfx");


    std::shared_ptr<gl::Texture> get_texture(const std::string & uri) {
      return tileset;
    }
    Vec2u get_tileset_size(const std::string & uri) {
      return Vec2u(16, 16);
    }
    Vec2u get_tileset_tile_size(const std::string & uri) {
      return Vec2u(16, 16);
    }


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


    void HUDOverlay::look(Vec2i screen_pos, const std::string & look_str) {
      this->look_pos = screen_pos;
      this->look_str = look_str;

      text_bin.set(*font_atlas, look_str);

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
      item.text_bin.set(*font_atlas, message);
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
      font_atlas.reset(new draw::FontAtlas);
      font_atlas->set_font(ttf_path);

      for(int i = 32 ; i < 128 ; i ++) {
        uint32_t code_point = i;
        font_atlas->load(code_point);
      }

      font_atlas->load_textures();

      gfx_topic.logf("font loaded from %s", ttf_path);
    }
    void load_tiles(const char * png_path) {
      Image tileset_image;
      load_png(tileset_image, png_path);
      tileset.reset(new gl::Texture(tileset_image));

      gfx_topic.logf("tiles loaded from %s", png_path);
      gfx_topic.logf("tileset->id(): %d", tileset->id());
    }

    void load() {
      tilemap_shader.reset(
          new TilemapShader(
            "#version 130\n\n"
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
            "}\n")); // lol
    }
    void unload() {
      tilemap_shader.reset();
      tileset.reset();
      font_atlas.reset();
    }
  }
}

