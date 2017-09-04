#ifndef RF_GFX_GL_TEXTURE_HPP
#define RF_GFX_GL_TEXTURE_HPP

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <rf/util/Image.hpp>

namespace rf {
  namespace gfx {
    namespace gl {
      class Texture {
        public:
        Texture();
        Texture(const Image & image);
        ~Texture();

        Texture(const Texture & other) = delete;
        Texture & operator=(const Texture & other) = delete;

        Texture(Texture && other);
        Texture & operator=(Texture && other);

        GLuint id() const { return _id; }
        unsigned int width() const { return _width; }
        unsigned int height() const { return _height; }

        private:
        unsigned int _width = 0;
        unsigned int _height = 0;
        GLuint _id = 0;

        void load(const Image & image);
      };
    }
  }
}

#endif
