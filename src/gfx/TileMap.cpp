
#include "TileMap.hpp"

#include <gfx/gl/Program.hpp>

namespace gfx {
  gl::Program TileMap::shader_program;
  gl::Texture TileMap::fg_color_tex;
  gl::Texture TileMap::bg_color_tex;
  gl::Texture TileMap::index_data_tex;
  GLint TileMap::fg_color_loc;
  GLint TileMap::bg_color_loc;
  GLint TileMap::index_data_loc;

  GLint TileMap::camera_pos_loc = 0;
  GLint TileMap::tile_set_loc = 0;

  void TileMap::set_screen_size(Vec2u pixels) {
    screen_size = pixels;
  }
  void TileMap::set_draw_rect(Rect2i pixels) {
    draw_rect = pixels;
  }
  void TileMap::set_tile_size(Vec2u pixels) {
    tile_size = pixels;
  }
  void TileMap::set_camera_pos(Vec2i pixels) {
    camera_pos = pixels;
  }

  void TileMap::set_size(Vec2u size) {
    if(size != _size) {
      _size = size;
      if(_size.x == 0 || _size.y == 0) {
        fg_color_data.release();
        bg_color_data.release();
        index_data.release();
      } else {
        fg_color_data = std::unique_ptr<uint8_t[]>(new uint8_t[_size.x * _size.y * 4]);
        bg_color_data = std::unique_ptr<uint8_t[]>(new uint8_t[_size.x * _size.y * 4]);
        index_data = std::unique_ptr<uint8_t[]>(new uint8_t[_size.x * _size.y * 3]);

        for(unsigned int i = 0 ; i < _size.x * _size.y ; i ++) {
          fg_color_data[4*i + 0] = rand();
          fg_color_data[4*i + 1] = rand();
          fg_color_data[4*i + 2] = rand();
          fg_color_data[4*i + 3] = 0xFF;

          bg_color_data[4*i + 0] = rand();
          bg_color_data[4*i + 1] = 0x00;
          bg_color_data[4*i + 2] = 0x00;
          bg_color_data[4*i + 3] = 0x7F;

          index_data[3*i + 0] = (i % _size.x) * 16;
          index_data[3*i + 1] = (i / _size.x) * 16;
          index_data[3*i + 2] = 0x00;
        }
      }
    }
  }
  void TileMap::set_tile(Vec2i tile, unsigned int idx) {
  }
  void TileMap::set_color(Vec2i tile, Color color) {
  }
  void TileMap::set_tile_set(const gl::Texture & texture) {
    tile_set_texid = texture.id();
  }

  void TileMap::draw() {
    if(shader_program.is_loaded() && size().x && size().y) {
      shader_program.use();

      glUniform2f(camera_pos_loc, camera_pos.x, camera_pos.y);

      glEnable(GL_VERTEX_ARRAY);
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tile_set_texid);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, fg_color_tex.id());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size().x, size().y, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, fg_color_data.get());

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, bg_color_tex.id());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size().x, size().y, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, bg_color_data.get());

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, index_data_tex.id());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size().x, size().y, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, index_data.get());

      glActiveTexture(GL_TEXTURE0);

      glUniform1i(tile_set_loc, 0);
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

      if(screen_size.x != 0 && screen_size.y != 0) {
        vertex_data[0] =  2.0f * ((float) (draw_rect.pos.x                   ) + 0.375f) / screen_size.x - 1.0f;
        vertex_data[1] = -2.0f * ((float) (draw_rect.pos.y                   ) + 0.375f) / screen_size.y + 1.0f;
        vertex_data[2] =  2.0f * ((float) (draw_rect.pos.x + draw_rect.size.x) + 0.375f) / screen_size.x - 1.0f;
        vertex_data[3] = -2.0f * ((float) (draw_rect.pos.y                   ) + 0.375f) / screen_size.y + 1.0f;
        vertex_data[4] =  2.0f * ((float) (draw_rect.pos.x + draw_rect.size.x) + 0.375f) / screen_size.x - 1.0f;
        vertex_data[5] = -2.0f * ((float) (draw_rect.pos.y + draw_rect.size.y) + 0.375f) / screen_size.y + 1.0f;
        vertex_data[6] =  2.0f * ((float) (draw_rect.pos.x                   ) + 0.375f) / screen_size.x - 1.0f;
        vertex_data[7] = -2.0f * ((float) (draw_rect.pos.y + draw_rect.size.y) + 0.375f) / screen_size.y + 1.0f;
      }

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertex_data + 0);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, vertex_data + 8);

      glDrawArrays(GL_QUADS, 0, 4);

      glDisable(GL_VERTEX_ARRAY);
      glDisableVertexAttribArray(1);
      glDisableVertexAttribArray(0);

      glUseProgram(0);
    }
  }

  bool TileMap::load_shader() {
    bool succ = false;

    using namespace gl;
    VertexShader vert;
    FragmentShader frag;
    if(vert.load("#version 130\n\n"
                 "in vec2 vertex_pos;\n"
                 "in vec2 vertex_texcoord;\n"
                 "uniform vec2 camera_pos;\n"
                 "varying vec2 texcoord;\n"
                 "void main() { gl_Position.xy = vertex_pos + camera_pos; gl_Position.z = 0.0; gl_Position.w = 1.0; texcoord = vertex_texcoord;}")) {
      if(frag.load("#version 130\n\n"
                   "varying vec2 texcoord;\n"
                   "uniform sampler2D tile_set;\n"
                   "uniform sampler2D fg_color;\n"
                   "uniform sampler2D bg_color;\n"
                   "uniform sampler2D index_data;\n"
                   "void main() { \n"
                   "vec4 fg = texture(fg_color, texcoord);\n"
                   "vec4 bg = texture(bg_color, texcoord);\n"
                   "vec2 tile_set_coord = texture(index_data, texcoord).xy * 255/256;\n"
                   "\n"
                   "vec2 tile_local_texcoord = texcoord - floor(texcoord*16)/16;\n"
                   "vec2 tile_set_texcoord = tile_set_coord + tile_local_texcoord*16/16;\n"
                   "vec4 tile_color = texture(tile_set, tile_set_texcoord);\n"
                   "\n"
                   "if(abs(tile_color.r - tile_color.g) < 0.001 && \n"
                   "   abs(tile_color.g - tile_color.b) < 0.001) {\n"
                   "gl_FragColor = bg + tile_color.r*(fg - bg);\n"
                   "} else {\n"
                   "gl_FragColor = tile_color;\n"
                   "}\n"
                   "}\n")) {
        shader_program.load();
        shader_program.attach(vert);
        shader_program.attach(frag);

        shader_program.bindAttribLocation(0, "vertex_pos");
        shader_program.bindAttribLocation(1, "vertex_texcoord");

        if(shader_program.link()) {
          printf("Successfully linked TileMap::shader_program\n");

          camera_pos_loc = shader_program.getUniformLocation("camera_pos");
          tile_set_loc = shader_program.getUniformLocation("tile_set");
          fg_color_loc = shader_program.getUniformLocation("fg_color");
          bg_color_loc = shader_program.getUniformLocation("bg_color");
          index_data_loc = shader_program.getUniformLocation("index_data");

          printf("shader_program.getAttribLocation(\"vertex_pos\"): %d\n",
              shader_program.getAttribLocation("vertex_pos"));
          printf("shader_program.getAttribLocation(\"vertex_texcoord\"): %d\n",
              shader_program.getAttribLocation("vertex_texcoord"));

          printf("shader_program.getUniformLocation(\"camera_pos\"): %d\n",
              shader_program.getUniformLocation("camera_pos"));
          printf("shader_program.getUniformLocation(\"tile_set\"): %d\n",
              shader_program.getUniformLocation("tile_set"));
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
  void TileMap::unload_shader() {
    fg_color_tex.unload();
    bg_color_tex.unload();
    index_data_tex.unload();

    shader_program.unload();
  }
}

