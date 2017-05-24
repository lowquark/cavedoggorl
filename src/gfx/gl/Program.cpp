
#include <gfx/gl/Program.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace gfx {
  namespace gl {
    VertexShader::~VertexShader() {
      if(id) {
        printf("Warning: Leaking vertex shader %u\n", id);
      }
    }
    bool VertexShader::is_loaded() {
      return id != 0;
    }
    bool VertexShader::load(const std::string & src) {
      if(!id) {
        id = glCreateShader(GL_VERTEX_SHADER);
      }

      const char * src_list[1] = { src.c_str() };
      glShaderSource(id, 1, src_list, 0);

      glCompileShader(id);

      GLint compiled;
      glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
      if(!compiled) {
        GLint blen = 0;	
        GLsizei slen = 0;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH , &blen);       
        if(blen > 1)
        {
          GLchar * compiler_log = (GLchar*)malloc(blen);
          glGetShaderInfoLog(id, blen, &slen, compiler_log);
          printf("Error compiling vertex shader: %.*s\n", slen, compiler_log);
          free(compiler_log);
        }

        return false;
      }

      return true;
    }
    void VertexShader::unload() {
      glDeleteShader(id);
      id = 0;
    }

    FragmentShader::~FragmentShader() {
      if(id) {
        printf("Warning: Leaking fragment shader %u\n", id);
      }
    }
    bool FragmentShader::is_loaded() {
      return id != 0;
    }
    bool FragmentShader::load(const std::string & src) {
      if(!id) {
        id = glCreateShader(GL_FRAGMENT_SHADER);
      }

      const char * src_list[1] = { src.c_str() };
      glShaderSource(id, 1, src_list, 0);

      glCompileShader(id);

      GLint compiled;
      glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
      if(!compiled) {
        GLint blen = 0;	
        GLsizei slen = 0;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH , &blen);       
        if(blen > 1)
        {
          GLchar * compiler_log = (GLchar*)malloc(blen);
          glGetShaderInfoLog(id, blen, &slen, compiler_log);
          printf("Error compiling fragment shader: %.*s\n", slen, compiler_log);
          free(compiler_log);
        }

        return false;
      }

      return true;
    }
    void FragmentShader::unload() {
      glDeleteShader(id);
      id = 0;
    }

    Program::~Program() {
      if(programId) {
        printf("Warning: Leaking program %u\n", programId);
      }
    }

    bool Program::is_loaded() {
      return programId != 0;
    }
    void Program::load() {
      if(!programId) {
        programId = glCreateProgram();
      }
    }
    void Program::attach(const VertexShader & vert) {
      if(programId) {
        glAttachShader(programId, vert.id);
      }
    }
    void Program::attach(const FragmentShader & frag) {
      if(programId) {
        glAttachShader(programId, frag.id);
      }
    }
    bool Program::link() {
      if(programId) {
        glLinkProgram(programId);

        GLint linked;
        glGetProgramiv(programId, GL_LINK_STATUS, &linked);
        if(!linked) {
          GLint blen = 0;	
          GLsizei slen = 0;

          glGetProgramiv(programId, GL_INFO_LOG_LENGTH , &blen);       
          if(blen > 1)
          {
            GLchar * compiler_log = (GLchar*)malloc(blen);
            glGetProgramInfoLog(programId, blen, &slen, compiler_log);
            printf("Error linking shader: %.*s\n", slen, compiler_log);
            free(compiler_log);
          }
        } else {
          return true;
        }
      }

      return false;
    }
    void Program::unload() {
      glDeleteProgram(programId);
      programId = 0;
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

