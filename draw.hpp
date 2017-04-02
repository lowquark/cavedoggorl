#ifndef DRAW_HPP
#define DRAW_HPP

#include "Entity.hpp"
#include "Color.hpp"
#include "BinPacker.hpp"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <map>
#include <memory>

static constexpr int TILE_WIDTH  = 32;
static constexpr int TILE_HEIGHT = 32;

static constexpr float WALL_WIDTH  = 28;
static constexpr float WALL_HEIGHT = 28;

static constexpr float FLOOR_DOT_WIDTH  = 4;
static constexpr float FLOOR_DOT_HEIGHT = 4;

static constexpr float AGENT_WIDTH  = 12;
static constexpr float AGENT_HEIGHT = 12;

static constexpr bool DRAW_PATHS = false;

static const Color WALL_COLOR(0.7f, 0.7f, 0.75f);
static const Color FLOOR_COLOR(0.3f, 0.3f, 0.3f);

namespace draw {
  extern Vec2f camera_pos;

  void draw_wall(Vec2i pos);
  void draw_floor(Vec2i pos);

  void draw_agent(Vec2i pos, const Color & color);
  void draw_agent(Vec2f pos, const Color & color);
  void draw_path(const std::vector<Vec2i> & path, const Color & color);

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
    const FontAtlas & _atlas;

    std::string _text;
    Rect2i _rect;

    std::vector<float> position_data; // 4 2D verts per glyph
    std::vector<float> texcoord_data; // 4 2D verts per glyph
    std::vector<GLuint> texture_data;

    public:
    TextBin(const FontAtlas & atlas)
      : _atlas(atlas) {}

    void set_text(const std::string & utf8);
    void set_rect(const Rect2i & rect);

    void update_layout();
    void draw();

    const FontAtlas & atlas() const { return _atlas; }

    const std::string & text() const { return _text; }
    const Rect2i & rect() const { return _rect; }
  };
}

#endif
