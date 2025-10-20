//-----------------------------------------------------------------------------
// ShaderProgram.h 
// GLSL shader manager class
//-----------------------------------------------------------------------------
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <map>
#define GLEW_STATIC
#include "GL/glew.h"
#include "glm/glm.hpp"
using std::string;

class ShaderProgram
{
public:
    ShaderProgram();
    ~ShaderProgram();

    enum ShaderType
    {
        VERTEX,
        FRAGMENT,
        PROGRAM
    };

    // Only supports vertex and fragment shaders
    bool loadShaders(const char* vsFilename, const char* fsFilename);
    void use();

    GLuint getProgram() const;

    // Uniform setting functions
    void setUniform(const GLchar* name, const bool b);
    void setUniform(const GLchar* name, const int i);
    void setUniform(const GLchar* name, const float f);
    void setUniform(const GLchar* name, const glm::vec2& v);
    void setUniform(const GLchar* name, const glm::vec3& v);
    void setUniform(const GLchar* name, const glm::vec4& v);
    void setUniform(const GLchar* name, const glm::mat4& m);

    // Speed up uniform lookups by caching locations
    GLint getUniformLocation(const GLchar * name);

private:
    string fileToString(const string& filename);
    void  checkCompileErrors(GLuint shader, ShaderType type);

    GLuint mHandle;
    std::map<string, GLint> mUniformLocations;
};
#endif // SHADER_H