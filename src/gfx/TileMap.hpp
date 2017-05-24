#ifndef GFX_TILEMAP_HPP
#define GFX_TILEMAP_HPP

#include <util/Vec2.hpp>
#include <util/Map.hpp>
#include <gfx/gl/Program.hpp>
#include <gfx/Color.hpp>
#include <memory>

namespace gfx {
  class TileMap {
    struct Tile {
      Color fg;
      Color bg;
      unsigned int idx;
    };

    Vec2u screen_size;
    Rect2i draw_rect;
    Vec2u tile_size;
    Vec2i camera_pos;

    GLuint tile_set_texid = 0;
    Vec2u _size;
    std::unique_ptr<uint8_t[]> fg_color_data;
    std::unique_ptr<uint8_t[]> bg_color_data;
    std::unique_ptr<uint8_t[]> index_data;


    static gl::Program shader_program;
    static gl::Texture fg_color_tex;
    static gl::Texture bg_color_tex;
    static gl::Texture index_data_tex;

    static GLint camera_pos_loc;
    static GLint tile_set_loc;
    static GLint fg_color_loc;
    static GLint bg_color_loc;
    static GLint index_data_loc;

    public:
    void set_screen_size(Vec2u pixels);
    void set_draw_rect(Rect2i pixels);
    void set_tile_size(Vec2u pixels);
    void set_camera_pos(Vec2i pixels);

    const Vec2u & size() const { return _size; }
    void set_size(Vec2u size);
    void set_tile(Vec2i tile, unsigned int idx);
    void set_color(Vec2i tile, Color color);
    void set_tile_set(const gl::Texture & texture);

    void draw();

    static bool load_shader();
    static void unload_shader();
  };
}

#endif
