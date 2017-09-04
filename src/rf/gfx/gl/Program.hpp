#ifndef RF_GFX_GL_PROGRAM_HPP
#define RF_GFX_GL_PROGRAM_HPP

#include <string>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

#include <rf/gfx/gl/Texture.hpp>

namespace rf {
  namespace gfx {
    namespace gl {
      class VertexShader {
        public:
        VertexShader();
        VertexShader(const std::string & src);
        ~VertexShader();

        VertexShader(const VertexShader & other) = delete;
        VertexShader & operator=(const VertexShader & other) = delete;

        VertexShader(VertexShader && other);
        VertexShader & operator=(VertexShader && other);

        bool compiled() { return _compiled; }
        bool compile(const std::string & src);

        private:
        GLuint id = 0;
        bool _compiled = false;

        friend class Program;
      };
      class FragmentShader {
        public:
        FragmentShader();
        FragmentShader(const std::string & src);
        ~FragmentShader();

        FragmentShader(const FragmentShader & other) = delete;
        FragmentShader & operator=(const FragmentShader & other) = delete;

        FragmentShader(FragmentShader && other);
        FragmentShader & operator=(FragmentShader && other);

        bool compiled() { return _compiled; }
        bool compile(const std::string & src);

        private:
        GLuint id = 0;
        bool _compiled = false;

        friend class Program;
      };

      // Considered a shader-program by opengl
      class Program {
        public:
        Program();
        ~Program();

        Program(const Program & other) = delete;
        Program & operator=(const Program & other) = delete;

        Program(Program && other);
        Program & operator=(Program && other);

        void attach(const VertexShader & vert);
        void attach(const FragmentShader & vert);
        void detach(const VertexShader & vert);
        void detach(const FragmentShader & vert);
        void detach_all();

        bool linked() { return _linked; }
        bool link();

        GLint getUniformLocation(const std::string & name);
        GLint getAttribLocation(const std::string & name);
        void bindAttribLocation(GLint loc, const std::string & name);

        void use();

        private:
        GLuint programId = 0;
        bool _linked = false;
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
}

#endif
