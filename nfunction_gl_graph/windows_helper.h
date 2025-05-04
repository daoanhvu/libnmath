#ifndef _WINDOWS_HELPER_H_
#define _WINDOWS_HELPER_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "shaders_utils.h"
#include "vbo_objects.h"


struct GLCoordinates {
  GLuint axisVao;
  GLuint axisVbo;
  GLuint axisColorVbo;
};

struct GLRotationParameters {
  glm::mat4 rotationMatrix = glm::mat4(1.0f); // Initialize as identity matrixßß
  // Variables for rotation
  float rotationX = 0.0f;
  float rotationY = 0.0f;
};

struct LightingParameters {
  glm::vec3 lightPos;
  glm::vec3 lightColor;
  glm::vec3 viewPos;
};

void handleRotation(GLFWwindow* window, GLRotationParameters &rotationParams);
GLCoordinates initCoordinates(ShaderVarLocation locations);
void renderCoordinates(const GLCoordinates &axes, ShaderVarLocation locations, glm::mat4 model);
void releaseCoordinates(GLCoordinates &axes);
void renderMeshes(std::vector<VboObject*>& meshes, const ShaderVarLocation &locations, const glm::mat4 &model, 
  int useLighting, const glm::vec3 &lightPos, const glm::vec3 &lightColor);

#endif