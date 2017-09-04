
#include "TileMap.hpp"

#include <gfx/draw.hpp>
#include <gfx/gl/Program.hpp>
#include <util/Log.hpp>

namespace gfx {
namespace draw {
  void TileMap::set_size(Vec2u size) {
    if(size != _size) {
      _size = size;
      if(_size.x == 0 || _size.y == 0) {
        indices.release();
        fg_color_data.release();
        bg_color_data.release();
        index_data.release();
      } else {
        indices = std::unique_ptr<unsigned int[]>(new unsigned int[_size.x * _size.y]);
        fg_color_data = std::unique_ptr<uint8_t[]>(new uint8_t[_size.x * _size.y * 4]);
        bg_color_data = std::unique_ptr<uint8_t[]>(new uint8_t[_size.x * _size.y * 4]);
        index_data = std::unique_ptr<uint8_t[]>(new uint8_t[_size.x * _size.y * 3]);

        for(unsigned int i = 0 ; i < _size.x * _size.y ; i ++) {
          indices[i] = 0;

          fg_color_data[4*i + 0] = 0xFF;
          fg_color_data[4*i + 1] = 0xFF;
          fg_color_data[4*i + 2] = 0xFF;
          fg_color_data[4*i + 3] = 0xFF;

          bg_color_data[4*i + 0] = 0x00;
          bg_color_data[4*i + 1] = 0x00;
          bg_color_data[4*i + 2] = 0x00;
          bg_color_data[4*i + 3] = 0xFF;

          index_data[3*i + 0] = 0x00;
          index_data[3*i + 1] = 0x00;
          index_data[3*i + 2] = 0x00;
        }
      }
    }
  }
  void TileMap::set_tile(Vec2i tile, unsigned int idx) {
    if(tile.x >= 0 && tile.x < _size.x &&
       tile.y >= 0 && tile.y < _size.y) {
      indices[tile.x + tile.y * _size.x] = idx;
    }
  }
  void TileMap::set_fg_color(Vec2i tile, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if(tile.x >= 0 && tile.x < _size.x &&
       tile.y >= 0 && tile.y < _size.y) {
      unsigned int i = tile.x + tile.y * _size.x;
      fg_color_data[4*i + 0] = r;
      fg_color_data[4*i + 1] = g;
      fg_color_data[4*i + 2] = b;
      fg_color_data[4*i + 3] = a;
    }
  }
  void TileMap::set_bg_color(Vec2i tile, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if(tile.x >= 0 && tile.x < _size.x &&
       tile.y >= 0 && tile.y < _size.y) {
      unsigned int i = tile.x + tile.y * _size.x;
      bg_color_data[4*i + 0] = r;
      bg_color_data[4*i + 1] = g;
      bg_color_data[4*i + 2] = b;
      bg_color_data[4*i + 3] = a;
    }
  }
  void TileMap::set_tileset(const gl::Texture & texture, Vec2u tileset_size, Vec2u tile_size) {
    tileset_texid = texture.id();
    _tileset_size = tileset_size;
    _tile_size = tile_size;

    extern Log log;
    if((256 / tileset_size.x) * tileset_size.x != 256 ||
       (256 / tileset_size.y) * tileset_size.y != 256) {
      log.logf<Log::WARNING>("%s: dimensions of tile set (%ux%u) are not both powers of 2",
                             __PRETTY_FUNCTION__,
                             tileset_size.x,
                             tileset_size.y);
    }

    // update indices now that tile set has been set

    for(unsigned int i = 0 ; i < _size.x * _size.y ; i ++) {
      index_data[3*i + 0] = (indices[i] % tileset_size.x) * 256 / tileset_size.x;
      index_data[3*i + 1] = (indices[i] / tileset_size.y) * 256 / tileset_size.y;
    }
  }

  void TileMapShader::draw(const TileMap & map, Vec2i pos) {
    if(shader_program.is_loaded() && map._size.x && map._size.y) {
      shader_program.use();

      glUniform2i(tilemap_size_loc, map._size.x, map._size.y);
      glUniform2i(tileset_size_loc, 16, 16);

      glEnable(GL_VERTEX_ARRAY);
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, map.tileset_texid);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, fg_color_tex.id());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map._size.x, map._size.y, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, map.fg_color_data.get());

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, bg_color_tex.id());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map._size.x, map._size.y, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, map.bg_color_data.get());

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, index_data_tex.id());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, map._size.x, map._size.y, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, map.index_data.get());

      glActiveTexture(GL_TEXTURE0);

      glUniform1i(tileset_loc, 0);
      glUniform1i(fg_color_loc, 1);
      glUniform1i(bg_color_loc, 2);
      glUniform1i(index_data_loc, 3);

      float vertex_data[16] = {
        // positions
        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
        -1.0f, -1.0f,
        // texcoords
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
      };

      draw::calc_quad(vertex_data, Rect2i(pos, map.draw_size()));

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertex_data + 0);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, vertex_data + 8);

      glDrawArrays(GL_QUADS, 0, 4);

      glDisable(GL_VERTEX_ARRAY);
      glDisableVertexAttribArray(1);
      glDisableVertexAttribArray(0);

      glUseProgram(0);
    }
  }

  bool TileMapShader::load(const std::string & vert_src, const std::string & frag_src) {
    bool succ = false;

    using namespace gl;
    VertexShader vert;
    FragmentShader frag;
    if(vert.load(vert_src)) {
      if(frag.load(frag_src)) {
        shader_program.load();
        shader_program.attach(vert);
        shader_program.attach(frag);

        shader_program.bindAttribLocation(0, "vertex_pos");
        shader_program.bindAttribLocation(1, "vertex_texcoord");

        if(shader_program.link()) {
          printf("Successfully linked TileMap::shader_program\n");

          tilemap_size_loc = shader_program.getUniformLocation("tilemap_size");
          tileset_size_loc = shader_program.getUniformLocation("tileset_size");

          tileset_loc = shader_program.getUniformLocation("tileset");
          fg_color_loc = shader_program.getUniformLocation("fg_color");
          bg_color_loc = shader_program.getUniformLocation("bg_color");
          index_data_loc = shader_program.getUniformLocation("index_data");

          printf("shader_program.getAttribLocation(\"vertex_pos\"): %d\n",
              shader_program.getAttribLocation("vertex_pos"));
          printf("shader_program.getAttribLocation(\"vertex_texcoord\"): %d\n",
              shader_program.getAttribLocation("vertex_texcoord"));

          printf("shader_program.getUniformLocation(\"tilemap_size\"): %d\n",
              shader_program.getUniformLocation("tilemap_size"));
          printf("shader_program.getUniformLocation(\"tileset_size\"): %d\n",
              shader_program.getUniformLocation("tileset_size"));

          printf("shader_program.getUniformLocation(\"tileset\"): %d\n",
              shader_program.getUniformLocation("tileset"));
          printf("shader_program.getUniformLocation(\"fg_color\"): %d\n",
              shader_program.getUniformLocation("fg_color"));
          printf("shader_program.getUniformLocation(\"bg_color\"): %d\n",
              shader_program.getUniformLocation("bg_color"));
          printf("shader_program.getUniformLocation(\"index_data\"): %d\n",
              shader_program.getUniformLocation("index_data"));
        } else {
          printf("Failed to link shader\n");
        }
      } else {
        printf("ERROR: Fragment shader failed to load\n");
      }
    } else {
      printf("ERROR: Vertex shader failed to load\n");
    }

    frag.unload();
    vert.unload();

    fg_color_tex.load();
    bg_color_tex.load();
    index_data_tex.load();

    printf("index_data_tex.id(): %u\n", index_data_tex.id());

    return succ;
  }
  void TileMapShader::unload() {
    fg_color_tex.unload();
    bg_color_tex.unload();
    index_data_tex.unload();

    shader_program.unload();
  }
}
}

