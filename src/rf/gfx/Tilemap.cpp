
#include "Tilemap.hpp"

#include <rf/gfx/draw.hpp>
#include <rf/util/Log.hpp>

namespace rf {
  namespace gfx {
    static LogTopic & gfx_topic = logtopic("gfx");

    extern std::shared_ptr<gl::Texture> get_texture(const std::string & uri);
    extern Vec2u get_tileset_size(const std::string & uri);
    extern Vec2u get_tileset_tile_size(const std::string & uri);

    TilemapData Tilemap::data(Vec2u tileset_size) const {
      /*
      if((256 / tileset_size.x) * tileset_size.x != 256 ||
         (256 / tileset_size.y) * tileset_size.y != 256) {
        printf("WARNING! %s: dimensions of tile set (%ux%u) are not both powers of 2",
               __PRETTY_FUNCTION__,
               tileset_size.x,
               tileset_size.y);
      }
      */

      TilemapData d;
      d.size = tiles.size();
      d.fg_color_data = std::unique_ptr<uint8_t[]>(new uint8_t[d.size.x * d.size.y * 4]);
      d.bg_color_data = std::unique_ptr<uint8_t[]>(new uint8_t[d.size.x * d.size.y * 4]);
      d.index_data = std::unique_ptr<uint8_t[]>(new uint8_t[d.size.x * d.size.y * 3]);

      unsigned int i = 0;
      for(unsigned int y = 0 ; y < tiles.size().y ; y ++) {
        for(unsigned int x = 0 ; x < tiles.size().x ; x ++) {
          auto & tile = tiles.get(Vec2u(x, y));
          d.fg_color_data[4*i + 0] = tile.foreground_color.r;
          d.fg_color_data[4*i + 1] = tile.foreground_color.g;
          d.fg_color_data[4*i + 2] = tile.foreground_color.b;
          d.fg_color_data[4*i + 3] = tile.foreground_color.a;

          d.bg_color_data[4*i + 0] = tile.background_color.r;
          d.bg_color_data[4*i + 1] = tile.background_color.g;
          d.bg_color_data[4*i + 2] = tile.background_color.b;
          d.bg_color_data[4*i + 3] = tile.background_color.a;

          d.index_data[3*i + 0] = (tile.tileset_index % tileset_size.x) * 256 / tileset_size.x;
          d.index_data[3*i + 1] = (tile.tileset_index / tileset_size.x) * 256 / tileset_size.y;

          i ++;
        }
      }

      return d;
    }


    TilemapShader::TilemapShader(const std::string & vert_src, const std::string & frag_src) {
      compile(vert_src, frag_src);
    }

    void TilemapShader::draw(const Tilemap & map, Vec2i pos) {
      std::shared_ptr<gl::Texture> tileset_tex = get_texture(map.tileset_uri);
      Vec2u tileset_size = get_tileset_size(map.tileset_uri);

      auto data = map.data(tileset_size);

      if(shader_program.linked()) {
        shader_program.use();

        glUniform2i(tilemap_size_loc, data.size.x, data.size.y);
        glUniform2i(tileset_size_loc, tileset_size.x, tileset_size.y);

        glEnable(GL_VERTEX_ARRAY);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glActiveTexture(GL_TEXTURE0);
        if(tileset_tex) {
          glBindTexture(GL_TEXTURE_2D, tileset_tex->id());
        } else {
          glBindTexture(GL_TEXTURE_2D, 0);
        }

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fg_color_tex.id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.size.x, data.size.y, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data.fg_color_data.get());

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, bg_color_tex.id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.size.x, data.size.y, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data.bg_color_data.get());

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, index_data_tex.id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.size.x, data.size.y, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, data.index_data.get());

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

        draw::calc_quad(vertex_data, Rect2i(pos, Vec2u(map.tiles.size().x*16, map.tiles.size().y*16)));

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertex_data + 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, vertex_data + 8);

        glDrawArrays(GL_QUADS, 0, 4);

        glDisable(GL_VERTEX_ARRAY);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);

        glUseProgram(0);
      }
    }

    bool TilemapShader::compile(const std::string & vert_src, const std::string & frag_src) {
      using namespace gl;
      VertexShader vert(vert_src);
      FragmentShader frag(frag_src);
      if(vert.compiled()) {
        if(frag.compiled()) {
          shader_program.detach_all();
          shader_program.attach(vert);
          shader_program.attach(frag);

          shader_program.bindAttribLocation(0, "vertex_pos");
          shader_program.bindAttribLocation(1, "vertex_texcoord");

          if(shader_program.link()) {
            gfx_topic.log("Successfully linked Tilemap::shader_program");

            tilemap_size_loc = shader_program.getUniformLocation("tilemap_size");
            tileset_size_loc = shader_program.getUniformLocation("tileset_size");

            tileset_loc = shader_program.getUniformLocation("tileset");
            fg_color_loc = shader_program.getUniformLocation("fg_color");
            bg_color_loc = shader_program.getUniformLocation("bg_color");
            index_data_loc = shader_program.getUniformLocation("index_data");

            gfx_topic.logf("shader_program.getAttribLocation(\"vertex_pos\"): %d",
                shader_program.getAttribLocation("vertex_pos"));
            gfx_topic.logf("shader_program.getAttribLocation(\"vertex_texcoord\"): %d",
                shader_program.getAttribLocation("vertex_texcoord"));

            gfx_topic.logf("shader_program.getUniformLocation(\"tilemap_size\"): %d",
                shader_program.getUniformLocation("tilemap_size"));
            gfx_topic.logf("shader_program.getUniformLocation(\"tileset_size\"): %d",
                shader_program.getUniformLocation("tileset_size"));

            gfx_topic.logf("shader_program.getUniformLocation(\"tileset\"): %d",
                shader_program.getUniformLocation("tileset"));
            gfx_topic.logf("shader_program.getUniformLocation(\"fg_color\"): %d",
                shader_program.getUniformLocation("fg_color"));
            gfx_topic.logf("shader_program.getUniformLocation(\"bg_color\"): %d",
                shader_program.getUniformLocation("bg_color"));
            gfx_topic.logf("shader_program.getUniformLocation(\"index_data\"): %d",
                shader_program.getUniformLocation("index_data"));
          } else {
            gfx_topic.warn("Failed to link shader");
          }
        } else {
          gfx_topic.warn("ERROR: Fragment shader failed to load");
        }
      } else {
        gfx_topic.warn("ERROR: Vertex shader failed to load");
      }

      gfx_topic.logf("index_data_tex.id(): %u", index_data_tex.id());

      return shader_program.linked();
    }
  }
}

