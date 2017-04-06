
#include "draw.hpp"

#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "Log.hpp"

namespace draw {
  static constexpr float WALL_WIDTH  = 28;
  static constexpr float WALL_HEIGHT = 28;

  static constexpr float FLOOR_DOT_WIDTH  = 4;
  static constexpr float FLOOR_DOT_HEIGHT = 4;

  static constexpr float AGENT_WIDTH  = 12;
  static constexpr float AGENT_HEIGHT = 12;

  static const Color WALL_COLOR(0.7f, 0.7f, 0.75f);
  static const Color FLOOR_COLOR(0.3f, 0.3f, 0.3f);

  static Vec2u tile_size(32, 32);
  void set_tile_size(const Vec2u & _tile_size) {
    tile_size = _tile_size;
  }

  static Vec2f camera_pos;
  void set_camera_pos(const Vec2f & _camera_pos) {
    camera_pos = _camera_pos;
  }

  static Log log;

  const float PIXEL_FUDGE = 0.375f;

  void draw_wall(Vec2i pos) {
    float x = (pos.x - camera_pos.x) * tile_size.x + PIXEL_FUDGE;
    float y = (pos.y - camera_pos.y) * tile_size.y + PIXEL_FUDGE;

    glTranslatef(x, y, 0.0f);

    glDisable(GL_TEXTURE_2D);
    glColor3f(WALL_COLOR.r, WALL_COLOR.g, WALL_COLOR.b);
    glBegin(GL_LINES);
      glVertex2f(-WALL_WIDTH/2, -WALL_HEIGHT/2);
      glVertex2f( WALL_WIDTH/2, -WALL_HEIGHT/2);

      glVertex2f( WALL_WIDTH/2, -WALL_HEIGHT/2);
      glVertex2f( WALL_WIDTH/2,  WALL_HEIGHT/2);

      glVertex2f( WALL_WIDTH/2,  WALL_HEIGHT/2);
      glVertex2f(-WALL_WIDTH/2,  WALL_HEIGHT/2);

      glVertex2f(-WALL_WIDTH/2,  WALL_HEIGHT/2);
      glVertex2f(-WALL_WIDTH/2, -WALL_HEIGHT/2);
    glEnd();

    glTranslatef(-x, -y, 0.0f);
  }
  void draw_floor(Vec2i pos) {
    float x = (pos.x - camera_pos.x) * tile_size.x + PIXEL_FUDGE;
    float y = (pos.y - camera_pos.y) * tile_size.y + PIXEL_FUDGE;

    glTranslatef(x, y, 0.0f);

    glDisable(GL_TEXTURE_2D);
    glColor3f(FLOOR_COLOR.r, FLOOR_COLOR.g, FLOOR_COLOR.b);
    glBegin(GL_QUADS);
      glVertex2f(-FLOOR_DOT_WIDTH/2, -FLOOR_DOT_HEIGHT/2);
      glVertex2f( FLOOR_DOT_WIDTH/2, -FLOOR_DOT_HEIGHT/2);
      glVertex2f( FLOOR_DOT_WIDTH/2,  FLOOR_DOT_HEIGHT/2);
      glVertex2f(-FLOOR_DOT_WIDTH/2,  FLOOR_DOT_HEIGHT/2);
    glEnd();

    glTranslatef(-x, -y, 0.0f);
  }

  void draw_agent(Vec2f pos, unsigned int type, const Color & color) {
    float x = (pos.x - camera_pos.x) * tile_size.x + PIXEL_FUDGE;
    float y = (pos.y - camera_pos.y) * tile_size.y + PIXEL_FUDGE;

    glTranslatef(x, y, 0.0f);

    glDisable(GL_TEXTURE_2D);
    glColor3f(color.r, color.g, color.b);
    if(type == 0) {
      glBegin(GL_QUADS);
        glVertex2f(-AGENT_WIDTH/2, -AGENT_HEIGHT/2);
        glVertex2f( AGENT_WIDTH/2, -AGENT_HEIGHT/2);
        glVertex2f( AGENT_WIDTH/2,  AGENT_HEIGHT/2);
        glVertex2f(-AGENT_WIDTH/2,  AGENT_HEIGHT/2);
      glEnd();
    } else if(type == 1) {
      glBegin(GL_TRIANGLES);
        glVertex2f( 0.0f,   0.0f);
        glVertex2f( (float)AGENT_WIDTH/2, -(float)AGENT_HEIGHT/2);
        glVertex2f( (float)AGENT_WIDTH/2,  (float)AGENT_HEIGHT/2);

        glVertex2f( 0.0f,   0.0f);
        glVertex2f(-(float)AGENT_WIDTH/2,  (float)AGENT_HEIGHT/2);
        glVertex2f(-(float)AGENT_WIDTH/2, -(float)AGENT_HEIGHT/2);
      glEnd();
    }

    glTranslatef(-x, -y, 0.0f);
  }
  void draw_path(const std::vector<Vec2i> & path, const Color & color) {
    if(path.size() > 0) {
      glDisable(GL_TEXTURE_2D);
      glColor3f(color.r, color.g, color.b);
      glBegin(GL_LINES);
        for(unsigned int i = 0 ; i < path.size() - 1 ; i ++) {
          float x0 = (path[i    ].x - camera_pos.x) * tile_size.x;
          float y0 = (path[i    ].y - camera_pos.y) * tile_size.y;
          float x1 = (path[i + 1].x - camera_pos.x) * tile_size.x;
          float y1 = (path[i + 1].y - camera_pos.y) * tile_size.y;

          glVertex2f(x0, y0);
          glVertex2f(x1, y1);
        }
      glEnd();
    }
  }

  void draw_rect(const Rect2i & rect, const Color & color) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
      glVertex2f(rect.pos.x,               rect.pos.y);
      glVertex2f(rect.pos.x + rect.size.x, rect.pos.y);
      glVertex2f(rect.pos.x + rect.size.x, rect.pos.y + rect.size.y);
      glVertex2f(rect.pos.x,               rect.pos.y + rect.size.y);
    glEnd();
  }

  static const Vec2u text_texture_size(512, 512);

  FontAtlas::FontAtlas() {
    _atlas_surface = SDL_CreateRGBSurfaceWithFormat(0, 512, 512, 32, SDL_PIXELFORMAT_RGBA8888);
    packer.set_size(Vec2u(512, 512));
  }
  FontAtlas::~FontAtlas() {
    if(_atlas_surface) {
      SDL_FreeSurface(_atlas_surface);
      _atlas_surface = nullptr;
    }
    if(font) {
      if(TTF_WasInit()) {
        TTF_CloseFont(font);
      }
      font = nullptr;
    }
  }

  void FontAtlas::set_font(const std::string & ttf_path) {
    if(font) {
      // because fuck all
      if(TTF_WasInit()) {
        TTF_CloseFont(font);
      }
      font = nullptr;
    }
    font = TTF_OpenFont(ttf_path.c_str(), 16);
  }

  bool FontAtlas::load(uint32_t unicode) {
    if(!font || !_atlas_surface) {
      return false;
    }

    bool succ = false;
    auto glyph_kvpair_it = glyphs.find(unicode);

    if(glyph_kvpair_it == glyphs.end()) {
      SDL_Color fg = { 0xFF, 0xFF, 0xFF, 0xFF };
      SDL_Surface * glyph_surface = TTF_RenderGlyph_Blended(font, unicode, fg);

      if(glyph_surface) {
        int min_x = 0;
        int max_x = 0;
        int min_y = 0;
        int max_y = 0;
        int advance_x = 0;

        TTF_GlyphMetrics(font, unicode, &min_x, &max_x, &min_y, &max_y, &advance_x);

        Rect2i bin_rect;
        if(packer.insert(bin_rect, Vec2u(glyph_surface->w, glyph_surface->h))) {
          SDL_Rect dst_rect;
          dst_rect.x = bin_rect.pos.x;
          dst_rect.y = bin_rect.pos.y;
          dst_rect.w = bin_rect.size.x;
          dst_rect.h = bin_rect.size.y;

          // Unlike the documentation specifies, NULL passed in place of a srcrect pointer did not work
          // https://wiki.libsdl.org/SDL_LowerBlit
          SDL_Rect src_rect;
          src_rect.x = 0;
          src_rect.y = 0;
          src_rect.w = glyph_surface->w;
          src_rect.h = glyph_surface->h;

          SDL_LowerBlit(glyph_surface, &src_rect, _atlas_surface, &dst_rect);

          Glyph new_glyph;
          new_glyph.advance_x = advance_x;
          new_glyph.atlas_tex_id = atlas_tex_id;
          new_glyph.rect = bin_rect;

          glyphs[unicode] = new_glyph;

          succ = true;
        }

        SDL_FreeSurface(glyph_surface);
        glyph_surface = nullptr;
      }
    } else {
      succ = true;
    }

    return succ;
  }
  // U+0x25AF is WHITE VERTICAL RECTANGLE 
  // IN CASE YOU WERE WONDERING
  bool FontAtlas::get(Rect2i & rect, int & advance_x, GLuint & atlas_tex_id, uint32_t unicode) const {
    auto glyph_kvpair_it = glyphs.find(unicode);

    if(glyph_kvpair_it != glyphs.end()) {
      rect = glyph_kvpair_it->second.rect;
      advance_x = glyph_kvpair_it->second.advance_x;
      atlas_tex_id = glyph_kvpair_it->second.atlas_tex_id;
      return true;
    }

    return false;
  }

  void FontAtlas::load_textures() {
    if(atlas_tex_id) {
      glDeleteTextures(1, &atlas_tex_id);
    }
    if(_atlas_surface) {
      glGenTextures(1, &atlas_tex_id);


      log.logf<Log::DEBUG0>("atlas_tex_id: %u", atlas_tex_id);
      log.logf<Log::DEBUG0>("_atlas_surface->w: %u", _atlas_surface->w);
      log.logf<Log::DEBUG0>("_atlas_surface->h: %u", _atlas_surface->h);

      glBindTexture(GL_TEXTURE_2D, atlas_tex_id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _atlas_surface->w, _atlas_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, _atlas_surface->pixels);

      for(auto & glyph_kvpair : glyphs) {
        glyph_kvpair.second.atlas_tex_id = atlas_tex_id;
      }
    }
  }
  void FontAtlas::unload_textures() {
    if(atlas_tex_id) {
      glDeleteTextures(1, &atlas_tex_id);
    }
  }

  // TODO: clear() should clear this stack
  static std::vector<Rect2i> clip_stack;

  void clip(const Rect2i & rect) {
    if(clip_stack.size() < 255) {
      // Increment the stencil buffer in this region, but only if previous stencil tests pass
      glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

      glBegin(GL_QUADS);
        glVertex2f(rect.pos.x,               rect.pos.y);
        glVertex2f(rect.pos.x + rect.size.x, rect.pos.y);
        glVertex2f(rect.pos.x + rect.size.x, rect.pos.y + rect.size.y);
        glVertex2f(rect.pos.x,               rect.pos.y + rect.size.y);
      glEnd();

      glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      clip_stack.push_back(rect);

      glStencilFunc(GL_EQUAL, clip_stack.size(), 0xFF);
    }
  }
  void unclip() {
    if(clip_stack.size()) {
      Rect2i & rect = clip_stack.back();
      // Decrement the stencil buffer in this region, but only if previous stencil tests pass
      glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

      glBegin(GL_QUADS);
        glVertex2f(rect.pos.x,               rect.pos.y);
        glVertex2f(rect.pos.x + rect.size.x, rect.pos.y);
        glVertex2f(rect.pos.x + rect.size.x, rect.pos.y + rect.size.y);
        glVertex2f(rect.pos.x,               rect.pos.y + rect.size.y);
      glEnd();

      glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      clip_stack.pop_back();
    }

    glStencilFunc(GL_EQUAL, clip_stack.size(), 0xFF);
  }

  void TextBin::set(const FontAtlas & atlas, const std::string & utf8_text) {
    _text_size = Vec2i(0, 0);
    Vec2i pos;

    position_data.clear();
    texcoord_data.clear();
    texture_data.clear();

    for(size_t i = 0 ; i < utf8_text.size() ; i ++) {
      Rect2i bin_rect;
      int advance_x;
      GLuint tex_id;

      if(utf8_text[i] == '\n') {
        pos.x = 0;
        pos.y += atlas.line_skip();
      } else if(atlas.get(bin_rect, advance_x, tex_id, utf8_text[i])) {
        int left   = pos.x;
        int right  = pos.x + bin_rect.size.x;
        int top    = pos.y;
        int bottom = pos.y + bin_rect.size.y;

        float uv_left   = (float)(bin_rect.pos.x) / text_texture_size.x;
        float uv_right  = (float)(bin_rect.pos.x + bin_rect.size.x) / text_texture_size.x;
        float uv_top    = (float)(bin_rect.pos.y) / text_texture_size.y;
        float uv_bottom = (float)(bin_rect.pos.y + bin_rect.size.y) / text_texture_size.y;

        position_data.push_back(left);
        position_data.push_back(top);

        position_data.push_back(right);
        position_data.push_back(top);

        position_data.push_back(right);
        position_data.push_back(bottom);

        position_data.push_back(left);
        position_data.push_back(bottom);

        texcoord_data.push_back(uv_left);
        texcoord_data.push_back(uv_top);

        texcoord_data.push_back(uv_right);
        texcoord_data.push_back(uv_top);

        texcoord_data.push_back(uv_right);
        texcoord_data.push_back(uv_bottom);

        texcoord_data.push_back(uv_left);
        texcoord_data.push_back(uv_bottom);

        texture_data.push_back(tex_id);

        if(right > _text_size.x) {
          _text_size.x = right;
        }
        if(bottom > _text_size.y) {
          _text_size.y = bottom;
        }

        pos += Vec2i(advance_x, 0);
      }
    }
  }
  void TextBin::draw() {
    assert(position_data.size() == texcoord_data.size());
    assert(position_data.size() == texture_data.size() * 8);

    draw::clip(_draw_rect);
    glTranslatef(_draw_rect.pos.x, _draw_rect.pos.y, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    if(texture_data.size()) {
      // Bind to first texture
      glBindTexture(GL_TEXTURE_2D, texture_data[0]);
      GLuint current_tex_id = texture_data[0];

      glBegin(GL_QUADS);

      for(int tex_i = 0 ; tex_i < texture_data.size() ; tex_i ++) {

        // Basic texture switch minimization
        if(texture_data[tex_i] != current_tex_id) {
          glEnd();

          glBindTexture(GL_TEXTURE_2D, texture_data[tex_i]);
          current_tex_id = texture_data[tex_i];

          glBegin(GL_QUADS);
        }

        // Very slow compared to an actual vbo
        glTexCoord2f(texcoord_data[8*tex_i    ], texcoord_data[8*tex_i + 1]);
          glVertex2f(position_data[8*tex_i    ], position_data[8*tex_i + 1]);
        glTexCoord2f(texcoord_data[8*tex_i + 2], texcoord_data[8*tex_i + 3]);
          glVertex2f(position_data[8*tex_i + 2], position_data[8*tex_i + 3]);
        glTexCoord2f(texcoord_data[8*tex_i + 4], texcoord_data[8*tex_i + 5]);
          glVertex2f(position_data[8*tex_i + 4], position_data[8*tex_i + 5]);
        glTexCoord2f(texcoord_data[8*tex_i + 6], texcoord_data[8*tex_i + 7]);
          glVertex2f(position_data[8*tex_i + 6], position_data[8*tex_i + 7]);
      }

      glEnd();
    }

    glTranslatef(-_draw_rect.pos.x, -_draw_rect.pos.y, 0.0f);
    draw::unclip();
  }
}

