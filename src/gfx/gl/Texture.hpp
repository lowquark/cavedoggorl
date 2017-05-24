#ifndef GFX_GL_TEXTURE_HPP
#define GFX_GL_TEXTURE_HPP

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <util/Image.hpp>

namespace gfx {
  namespace gl {
    class Texture {
      unsigned int _width = 0;
      unsigned int _height = 0;
      GLuint _id = 0;

      public:
      Texture() = default;
      ~Texture();

      Texture(const Texture & other) = delete;
      Texture & operator=(const Texture & other) = delete;

      Texture(Texture && other) {
        _id = other._id;
        other._id = 0;
      }

      void load();
      void load(const Image & image);
      void unload();

      GLuint id() const { return _id; }
      unsigned int width() const { return _width; }
      unsigned int height() const { return _height; }
    };
  }
}

#endif
