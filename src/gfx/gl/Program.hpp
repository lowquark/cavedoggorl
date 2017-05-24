#ifndef GFX_GL_PROGRAM_HPP
#define GFX_GL_PROGRAM_HPP

#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

#include <gfx/gl/Texture.hpp>

namespace gfx {
  namespace gl {
    class VertexShader {
      public:
      VertexShader()
        : id(0) {}
      ~VertexShader();

      VertexShader(const VertexShader & other) = delete;
      VertexShader & operator=(const VertexShader & other) = delete;

      VertexShader(VertexShader && other) {
        id = other.id;
        other.id = 0;
      }

      bool is_loaded();
      bool load(const std::string & src);
      void unload();

      private:
      GLuint id;

      friend class Program;
    };
    class FragmentShader {
      public:
      FragmentShader()
        : id(0) {}
      ~FragmentShader();

      FragmentShader(const FragmentShader & other) = delete;
      FragmentShader & operator=(const FragmentShader & other) = delete;

      FragmentShader(FragmentShader && other) {
        id = other.id;
        other.id = 0;
      }

      bool is_loaded();
      bool load(const std::string & src);
      void unload();

      private:
      GLuint id;

      friend class Program;
    };

    // Considered a shader-program by opengl
    class Program {
      public:
      Program()
        : programId(0) {}
      ~Program();

      Program(const Program & other) = delete;
      Program & operator=(const Program & other) = delete;

      Program(Program && other) {
        programId = other.programId;
        other.programId = 0;
      }

      static constexpr GLint GG_POSITION_LOCATION = 0;
      static constexpr GLint GG_NORMAL_LOCATION   = 1;
      static constexpr GLint GG_COLOR_LOCATION    = 2;
      static constexpr GLint GG_TEXCOORD_LOCATION = 3;

      bool is_loaded();
      void load();
      void attach(const VertexShader & vert);
      void attach(const FragmentShader & vert);
      bool link();
      void unload();

      GLint getUniformLocation(const std::string & name);
      GLint getAttribLocation(const std::string & name);
      void bindAttribLocation(GLint loc, const std::string & name);

      void use();

      private:
      GLuint programId;
    };

    // TODO: Array versions of these
    void setUniform(GLint location, GLfloat v);
    void setUniform(GLint location, const glm::vec2 & v);
    void setUniform(GLint location, const glm::vec3 & v);
    void setUniform(GLint location, const glm::vec4 & v);

    void setUniform(GLint location, GLint v); 
    void setUniform(GLint location, const glm::ivec2 & v);
    void setUniform(GLint location, const glm::ivec3 & v);
    void setUniform(GLint location, const glm::ivec4 & v);

    void setUniform(GLint location, GLuint v);
    void setUniform(GLint location, const glm::uvec2 & v);
    void setUniform(GLint location, const glm::uvec3 & v);
    void setUniform(GLint location, const glm::uvec4 & v);

    void setUniform(GLint location, const glm::mat2 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat3 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat4 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat2x3 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat3x2 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat2x4 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat4x2 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat3x4 & m, GLboolean transpose = GL_FALSE);
    void setUniform(GLint location, const glm::mat4x3 & m, GLboolean transpose = GL_FALSE);
  }
}

#endif
