
#include <cstdio>

#include <gfx/gl/Texture.hpp>

namespace gfx {
  namespace gl {
    Texture::~Texture() {
      if(_id) {
        printf("Warning: Leaking texture %u\n", _id);
      }
    }
    void Texture::load() {
      if(_id == 0) {
        glGenTextures(1, &_id);
      }
    }
    void Texture::load(const Image & image) {
      _width = image.width();
      _height = image.height();

      if(image.pixels()) {
        load();
        glBindTexture(GL_TEXTURE_2D, _id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels());
        // TODO: Rebind old texture
        glBindTexture(GL_TEXTURE_2D, 0);
      } else {
        unload();
      }
    }
    void Texture::unload() {
      glDeleteTextures(1, &_id);
      _id = 0;
    }
  }
}

