#ifndef DRAW_HPP
#define DRAW_HPP

#include <map>
#include <vector>

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <gfx/Color.hpp>
#include <gfx/BinPacker.hpp>

namespace gfx {
  namespace draw {
    void set_tile_size(const Vec2u & tile_size);
    void set_camera_pos(const Vec2f & camera_pos);

    void draw_wall(Vec2i pos, Color filter = Color(1.0f, 1.0f, 1.0f));
    void draw_floor(Vec2i pos, Color filter = Color(1.0f, 1.0f, 1.0f));

    void draw_dark_wall(Vec2i pos);
    void draw_dark_floor(Vec2i pos);

    void draw_agent(Vec2f pos, unsigned int type_id, const Color & color);
    void draw_path(const std::vector<Vec2i> & path, const Color & color);

    void draw_rect(const Rect2i & rect, const Color & color);

    class FontAtlas {
      TTF_Font * font = nullptr;

      // TODO: Load more of these as they become full
      BinPacker packer;
      SDL_Surface * _atlas_surface = nullptr;
      GLuint atlas_tex_id = 0;

      // Data for drawing a particular glyph
      struct Glyph {
        int advance_x = 0;
        GLuint atlas_tex_id = 0;
        Rect2i rect;
      };

      std::map<uint32_t, Glyph> glyphs;

      public:
      FontAtlas();
      FontAtlas(const FontAtlas & other) = delete;
      FontAtlas & operator=(const FontAtlas & other) = delete;
      ~FontAtlas();

      void set_font(const std::string & ttf_path);

      bool load(uint32_t code_point);
      bool get(Rect2i & rect, int & advance_x, GLuint & atlas_tex_id, uint32_t code_point) const;
      void load_textures();
      void unload_textures();

      int line_skip() const {
        if(font) {
          return TTF_FontLineSkip(font);
        } else {
          return 0;
        }
      }

      SDL_Surface * atlas_surface() const { return _atlas_surface; }
    };

    // Recursive clipping
    void clip(const Rect2i & rect);
    void unclip();

    class TextBin {
      Vec2i _text_size;
      Rect2i _draw_rect;

      std::vector<float> position_data; // 4 2D verts per glyph
      std::vector<float> texcoord_data; // 4 2D verts per glyph
      std::vector<GLuint> texture_data;

      public:
      void set(const FontAtlas & atlas, const std::string & utf8_text);
      void set(const FontAtlas & atlas, const std::string & utf8_text, const Vec2u & wrap_size);

      // post-set text size
      const Vec2i & text_size() const { return _text_size; }

      const Rect2i & draw_rect() const { return _draw_rect; }
      void set_draw_rect(const Rect2i & draw_rect) { _draw_rect = draw_rect; }

      void draw();
    };
  }
}

#endif
