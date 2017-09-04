
#include "Program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <rf/util/Log.hpp>

namespace rf {
  namespace gfx {
    namespace gl {
      static LogTopic & gl_topic = logtopic("gl");

      VertexShader::VertexShader() {
        id = glCreateShader(GL_VERTEX_SHADER);
      }
      VertexShader::VertexShader(const std::string & src) {
        id = glCreateShader(GL_VERTEX_SHADER);

        compile(src);
      }
      VertexShader::~VertexShader() {
        glDeleteShader(id);
        id = 0;
      }

      VertexShader::VertexShader(VertexShader && other) {
        id = other.id;
        _compiled = other._compiled;
        other.id = 0;
        other._compiled = false;
      }
      VertexShader & VertexShader::operator=(VertexShader && other) {
        if(this != &other) {
          glDeleteShader(id);

          id = other.id;
          _compiled = other._compiled;
          other.id = 0;
          other._compiled = false;
        }

        return *this;
      }

      bool VertexShader::compile(const std::string & src) {
        const char * src_list[1] = { src.c_str() };
        glShaderSource(id, 1, src_list, 0);

        glCompileShader(id);

        GLint compile_status;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compile_status);

        if(!compile_status) {
          GLint blen = 0;	
          GLsizei slen = 0;

          glGetShaderiv(id, GL_INFO_LOG_LENGTH , &blen);       
          if(blen > 1) {
            GLchar * compiler_log = (GLchar*)malloc(blen);
            glGetShaderInfoLog(id, blen, &slen, compiler_log);
            gl_topic.warnf("Error compiling vertex shader: %.*s", slen, compiler_log);
            free(compiler_log);
          }
        }

        _compiled = compile_status;

        return _compiled;
      }

      FragmentShader::FragmentShader() {
        id = glCreateShader(GL_FRAGMENT_SHADER);
      }
      FragmentShader::FragmentShader(const std::string & src) {
        id = glCreateShader(GL_FRAGMENT_SHADER);

        compile(src);
      }
      FragmentShader::~FragmentShader() {
        glDeleteShader(id);
        id = 0;
      }

      FragmentShader::FragmentShader(FragmentShader && other) {
        id = other.id;
        _compiled = other._compiled;
        other.id = 0;
        other._compiled = false;
      }
      FragmentShader & FragmentShader::operator=(FragmentShader && other) {
        if(this != &other) {
          glDeleteShader(id);

          id = other.id;
          _compiled = other._compiled;
          other.id = 0;
          other._compiled = false;
        }

        return *this;
      }

      bool FragmentShader::compile(const std::string & src) {
        const char * src_list[1] = { src.c_str() };
        glShaderSource(id, 1, src_list, 0);

        glCompileShader(id);

        GLint compile_status;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compile_status);

        if(!compile_status) {
          GLint blen = 0;	
          GLsizei slen = 0;

          glGetShaderiv(id, GL_INFO_LOG_LENGTH , &blen);       
          if(blen > 1) {
            GLchar * compiler_log = (GLchar*)malloc(blen);
            glGetShaderInfoLog(id, blen, &slen, compiler_log);
            gl_topic.warnf("Error compiling vertex shader: %.*s", slen, compiler_log);
            free(compiler_log);
          }
        }

        _compiled = compile_status;

        return _compiled;
      }

      Program::Program() {
        programId = glCreateProgram();
      }
      Program::~Program() {
        glDeleteProgram(programId);
        programId = 0;
      }

      Program::Program(Program && other) {
        programId = other.programId;
        _linked = other._linked;
        other.programId = 0;
        other._linked = false;
      }
      Program & Program::operator=(Program && other) {
        if(this != &other) {
          glDeleteProgram(programId);

          programId = other.programId;
          _linked = other._linked;
          other.programId = 0;
          other._linked = false;
        }

        return *this;
      }

      void Program::attach(const VertexShader & vert) {
        glAttachShader(programId, vert.id);
      }
      void Program::attach(const FragmentShader & frag) {
        glAttachShader(programId, frag.id);
      }
      void Program::detach(const VertexShader & vert) {
        glDetachShader(programId, vert.id);
      }
      void Program::detach(const FragmentShader & frag) {
        glDetachShader(programId, frag.id);
      }
      void Program::detach_all() {
        GLint num_shaders = 0;
        glGetProgramiv(programId, GL_ATTACHED_SHADERS, &num_shaders);

        if(num_shaders) {
          GLuint * shaders = new GLuint[num_shaders];
          glGetAttachedShaders(programId, num_shaders, NULL, shaders);

          for(int i = 0 ; i < num_shaders ; i ++) {
            glDetachShader(programId, shaders[i]);
          }

          delete [] shaders;
          shaders = nullptr;
        }
      }
      bool Program::link() {
        glLinkProgram(programId);

        GLint link_status;
        glGetProgramiv(programId, GL_LINK_STATUS, &link_status);
        if(!link_status) {
          GLint blen = 0;	
          GLsizei slen = 0;

          glGetProgramiv(programId, GL_INFO_LOG_LENGTH , &blen);       
          if(blen > 1)
          {
            GLchar * compiler_log = (GLchar*)malloc(blen);
            glGetProgramInfoLog(programId, blen, &slen, compiler_log);
            gl_topic.warnf("Error linking shader: %.*s", slen, compiler_log);
            free(compiler_log);
          }
        }

        _linked = link_status;
        return _linked;
      }

      GLint Program::getUniformLocation(const std::string & name) {
        return glGetUniformLocation(programId, name.c_str());
      }
      GLint Program::getAttribLocation(const std::string & name) {
        return glGetAttribLocation(programId, name.c_str());
      }
      void Program::bindAttribLocation(GLint loc, const std::string & name) {
        glBindAttribLocation(programId, loc, name.c_str());
      }

      void Program::use() {
        glUseProgram(programId);
      }

      void setUniform(GLint location, GLfloat v) {
        glUniform1f(location, v);
      }
      void setUniform(GLint location, const glm::vec2 & v) {
        glUniform2fv(location, 1, glm::value_ptr(v));
      }
      void setUniform(GLint location, const glm::vec3 & v) {
        glUniform3fv(location, 1, glm::value_ptr(v));
      }
      void setUniform(GLint location, const glm::vec4 & v) {
        glUniform4fv(location, 1, glm::value_ptr(v));
      }

      void setUniform(GLint location, GLint v) {
        glUniform1i(location, v);
      }
      void setUniform(GLint location, const glm::ivec2 & v) {
        glUniform2iv(location, 1, glm::value_ptr(v));
      }
      void setUniform(GLint location, const glm::ivec3 & v) {
        glUniform3iv(location, 1, glm::value_ptr(v));
      }
      void setUniform(GLint location, const glm::ivec4 & v) {
        glUniform4iv(location, 1, glm::value_ptr(v));
      }

      void setUniform(GLint location, GLuint v) {
        glUniform1ui(location, v);
      }
      void setUniform(GLint location, const glm::uvec2 & v) {
        glUniform2uiv(location, 1, glm::value_ptr(v));
      }
      void setUniform(GLint location, const glm::uvec3 & v) {
        glUniform3uiv(location, 1, glm::value_ptr(v));
      }
      void setUniform(GLint location, const glm::uvec4 & v) {
        glUniform4uiv(location, 1, glm::value_ptr(v));
      }

      void setUniform(GLint location, const glm::mat2 & m, GLboolean transpose) {
        glUniformMatrix2fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat3 & m, GLboolean transpose) {
        glUniformMatrix3fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat4 & m, GLboolean transpose) {
        glUniformMatrix4fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat2x3 & m, GLboolean transpose) {
        glUniformMatrix2x3fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat3x2 & m, GLboolean transpose) {
        glUniformMatrix3x2fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat2x4 & m, GLboolean transpose) {
        glUniformMatrix2x4fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat4x2 & m, GLboolean transpose) {
        glUniformMatrix4x2fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat3x4 & m, GLboolean transpose) {
        glUniformMatrix3x4fv(location, 1, transpose, glm::value_ptr(m));
      }
      void setUniform(GLint location, const glm::mat4x3 & m, GLboolean transpose) {
        glUniformMatrix4x3fv(location, 1, transpose, glm::value_ptr(m));
      }
    }
  }
}

