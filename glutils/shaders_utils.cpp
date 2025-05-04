#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


std::string readShaderFile(const char* filePath) {
  std::string shaderCode;
  std::ifstream shaderFile;
  
  // Ensure ifstream objects can throw exceptions
  shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
  try {
    // Open file
    shaderFile.open(filePath);
    std::stringstream shaderStream;
    
    // Read file's buffer contents into stream
    shaderStream << shaderFile.rdbuf();
    
    // Close file handler
    shaderFile.close();
    
    // Convert stream into string
    shaderCode = shaderStream.str();
  }
  catch(std::ifstream::failure& e) {
      std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
  }
  
  return shaderCode;
}

// Function to compile shader
GLuint compileShader(const char* source, GLenum shaderType) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  
  // Check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  
  return shader;
}