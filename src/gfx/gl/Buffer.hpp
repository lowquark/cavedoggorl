#ifndef GFX_BUFFER_HPP
#define GFX_BUFFER_HPP

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

namespace gfx {
  namespace gl {
    class ArrayBuffer {
      GLuint name;

      public:
      ArrayBuffer()
        : name(0) {}

      ArrayBuffer(const ArrayBuffer & other) = delete;
      ArrayBuffer & operator=(const ArrayBuffer & other) = delete;

      ArrayBuffer(ArrayBuffer && other) {
        name = other.name;
        other.name = 0;
      }

      void load(const GLvoid * data, GLsizeiptr dataSize, GLenum usage = GL_STATIC_DRAW) {
        if(!name) {
          glGenBuffers(1, &name);
        }
        GLint oldName;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldName);
        glBindBuffer(GL_ARRAY_BUFFER, name);
        glBufferData(GL_ARRAY_BUFFER, dataSize, data, usage);
        glBindBuffer(GL_ARRAY_BUFFER, oldName);
      }
      void unload() {
        glDeleteBuffers(1, &name);
        name = 0;
      }

      void bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, name);
      }
      static void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }

      //friend void bindBuffer(const ArrayBuffer & buffer);
    };
    class ElementArrayBuffer {
      GLuint name;

      public:
      ElementArrayBuffer()
        : name(0) {}

      ElementArrayBuffer(const ElementArrayBuffer & other) = delete;
      ElementArrayBuffer & operator=(const ElementArrayBuffer & other) = delete;

      ElementArrayBuffer(ElementArrayBuffer && other) {
        name = other.name;
        other.name = 0;
      }

      void load(const GLvoid * data, GLsizeiptr dataSize, GLenum usage = GL_STATIC_DRAW) {
        if(!name) {
          glGenBuffers(1, &name);
        }
        GLint oldName;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldName);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, name);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, usage);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldName);
      }
      void unload() {
        glDeleteBuffers(1, &name);
        name = 0;
      }

      void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, name);
      }
      static void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }

      //friend void draw(const ElementArrayBuffer & buffer);
    };

    void draw(GLenum mode, GLsizei count);
    void draw(const ElementArrayBuffer & ebuf, GLenum mode, GLsizei count);
  }
}

#endif
