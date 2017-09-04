#ifndef GFX_TILEMAP_HPP
#define GFX_TILEMAP_HPP

#include <util/Vec2.hpp>
#include <util/Map.hpp>
#include <gfx/gl/Program.hpp>
#include <gfx/Color.hpp>
#include <memory>

namespace gfx {
namespace draw {
  class TileMap {
    public:
    const Vec2u & size() const { return _size; }
    void set_size(Vec2u size);

    Vec2u draw_size() const { return Vec2u(_size.x*_tile_size.x,
                                           _size.y*_tile_size.y); }

    void set_tileset(const gl::Texture & texture, Vec2u tileset_size, Vec2u tile_size);
    void set_tile(Vec2i tile, unsigned int idx);
    void set_fg_color(Vec2i tile, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);
    void set_bg_color(Vec2i tile, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF);

    private:
    Vec2u _size; // size in tiles

    GLuint tileset_texid = 0; // tileset texture
    Vec2u _tileset_size;      // tileset size in tiles
    Vec2u _tile_size;          // tileset tile size in pixels

    // interim index data
    std::unique_ptr<unsigned int[]> indices;

    // uploaded to GPU as textures
    std::unique_ptr<uint8_t[]> fg_color_data;
    std::unique_ptr<uint8_t[]> bg_color_data;
    std::unique_ptr<uint8_t[]> index_data;

    friend class TileMapShader;
  };
  class TileMapShader {
    public:
    void draw(const TileMap & map, Vec2i pos);

    bool load(const std::string & vert_src, const std::string & frag_src);
    void unload();

    private:
    gl::Program shader_program;
    gl::Texture fg_color_tex;
    gl::Texture bg_color_tex;
    gl::Texture index_data_tex;

    GLint tilemap_size_loc;
    GLint tileset_size_loc;

    GLint tileset_loc;
    GLint fg_color_loc;
    GLint bg_color_loc;
    GLint index_data_loc;
  };
}
}

#endif
