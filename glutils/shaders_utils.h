#include <string>
#include <GL/glew.h>

std::string readShaderFile(const char* filePath);
GLuint compileShader(const char* source, GLenum shaderType);