#ifndef RF_GFX_TILEMAP_HPP
#define RF_GFX_TILEMAP_HPP

#include <rf/util/Vec2.hpp>
#include <rf/util/Map.hpp>
#include <rf/gfx/gl/Program.hpp>
#include <memory>

namespace rf {
  namespace gfx {
    struct TilemapData {
      // uploaded to GPU as textures
      std::unique_ptr<uint8_t[]> fg_color_data;
      std::unique_ptr<uint8_t[]> bg_color_data;
      std::unique_ptr<uint8_t[]> index_data;
      Vec2u size;
    };

    class Tilemap {
      public:
      struct Color {
        uint8_t r = 0x00;
        uint8_t g = 0x00;
        uint8_t b = 0x00;
        uint8_t a = 0x00;

        Color() = default;
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) noexcept
          : r(r), g(g), b(b), a(a) {}

        Color & operator+=(const Color & other) noexcept {
          this->r += other.r;
          this->g += other.g;
          this->b += other.b;
          return *this;
        }
        Color operator+(const Color & other) const noexcept {
          return Color(*this) += other;
        }

        Color & operator-=(const Color & other) noexcept {
          this->r -= other.r;
          this->g -= other.g;
          this->b -= other.b;
          return *this;
        }
        Color operator-(const Color & other) const noexcept {
          return Color(*this) -= other;
        }
      };

      struct Tile {
        unsigned int tileset_index;
        Color foreground_color;
        Color background_color;
      };

      std::string tileset_uri;
      Map<Tile> tiles;

      TilemapData data(Vec2u tileset_size) const;
    };

    class TilemapShader {
      public:
      TilemapShader() = default;
      TilemapShader(const std::string & vert_src, const std::string & frag_src);

      TilemapShader(const TilemapShader & other) = delete;
      TilemapShader & operator=(const TilemapShader & other) = delete;

      void draw(const Tilemap & map, Vec2i pos);

      bool compile(const std::string & vert_src, const std::string & frag_src);
      bool compiled() { return shader_program.linked(); };

      private:
      gl::Program shader_program;
      gl::Texture fg_color_tex;
      gl::Texture bg_color_tex;
      gl::Texture index_data_tex;

      GLint tilemap_size_loc = 0;
      GLint tileset_size_loc = 0;

      GLint tileset_loc = 0;
      GLint fg_color_loc = 0;
      GLint bg_color_loc = 0;
      GLint index_data_loc = 0;
    };
  }
}

#endif
