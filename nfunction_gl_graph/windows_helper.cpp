#include "windows_helper.h"

// In your main rendering loop
void handleRotation(GLFWwindow* window, GLRotationParameters &rotationParams) {
  // Create rotation matrices using the accumulated angles
  glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(rotationParams.rotationX), glm::vec3(1.0f, 0.0f, 0.0f));  // Fixed X-axis
  glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(rotationParams.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));  // Fixed Y-axis
  
  // Apply rotations in order: first around Y-axis, then around X-axis
  rotationParams.rotationMatrix = rotX * rotY;
}

GLCoordinates initCoordinates(ShaderVarLocation locations) {
  // Initialize the coordinate system here
  // This function can be used to set up any initial state for the coordinate system
  float axisVertices[] = {
      // X-axis
      -1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      // Y-axis
      0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      // Z-axis
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, 1.0f
  };
  float axisColors[] = {
      // X-axis color (red)
      1.0f, 0.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 0.0f, 1.0f,
      // Y-axis color (green)
      0.0f, 1.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f, 1.0f,
      // Z-axis color (blue)
      0.0f, 0.0f, 1.0f, 1.0f,
      0.0f, 0.0f, 1.0f, 1.0f
  };
  // VboObject* axisVBO = new VboObject(locations, GL_LINES);
  // axisVBO->initialize(axisVertices, 18, 6, axisColors, nullptr, 0, -1);
  GLCoordinates result;
  GLuint axisVao;
  GLuint axisVbo;
  GLuint axisColorVbo;
  glGenVertexArrays(1, &axisVao);
  glBindVertexArray(axisVao);

  glGenBuffers(1, &axisVbo);
  glBindBuffer(GL_ARRAY_BUFFER, axisVbo);
  glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), axisVertices, GL_STATIC_DRAW);
  // Set up position attribute (location 0)
  glBindBuffer(GL_ARRAY_BUFFER, axisVbo);
  int strideInBytes = 3 * sizeof(float);
  glVertexAttribPointer(locations.positionLocation, 3, GL_FLOAT, GL_FALSE, strideInBytes, (void*)0);
  glEnableVertexAttribArray(locations.positionLocation);

  strideInBytes = 4 * sizeof(float);
  glGenBuffers(1, &axisColorVbo);
  glBindBuffer(GL_ARRAY_BUFFER, axisColorVbo);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), axisColors, GL_STATIC_DRAW);

  result.axisVao = axisVao;
  result.axisVbo = axisVbo;
  result.axisColorVbo = axisColorVbo;
  return result;
}

void renderCoordinates(const GLCoordinates &axes, ShaderVarLocation locations, glm::mat4 model) {
  glUniform1i(locations.useLightingLocation, 0);
  glUniformMatrix4fv(locations.modelMatrixId, 1, GL_FALSE, glm::value_ptr(model));
  // Render the coordinate system here
  glBindVertexArray(axes.axisVao);

  glBindBuffer(GL_ARRAY_BUFFER, axes.axisVbo);
  glEnableVertexAttribArray(locations.positionLocation);
  glVertexAttribPointer(locations.positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        3 * sizeof(float),
                        reinterpret_cast<void*>(0));

  glBindBuffer(GL_ARRAY_BUFFER, axes.axisColorVbo);
  glEnableVertexAttribArray(locations.colorLocation);
  glVertexAttribPointer(locations.colorLocation, //Attribute index
                        4,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        4 * sizeof(float),
                        reinterpret_cast<void*>(0));
  
    glDrawArrays(GL_LINES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisableVertexAttribArray(locations.positionLocation);
    glDisableVertexAttribArray(locations.normalLocation);
    glDisableVertexAttribArray(locations.colorLocation);
}

void releaseCoordinates(GLCoordinates &axes) {
  glDeleteVertexArrays(1, &axes.axisVao);
  glDeleteBuffers(1, &axes.axisVbo);
  glDeleteBuffers(1, &axes.axisColorVbo);
}

void renderMeshes(std::vector<VboObject*>& meshes, const ShaderVarLocation &locations, const glm::mat4 &model, 
      int useLighting, const glm::vec3 &lightPos, const glm::vec3 &lightColor) {
  // Set lighting uniforms
  glUniform1i(locations.useLightingLocation, useLighting);
  glUniform3fv(locations.lightPos1ID, 1, glm::value_ptr(lightPos));
  glUniform3fv(locations.lightColor1ID, 1, glm::value_ptr(lightColor));

  // Calculate and set normal matrix
  glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
  glUniformMatrix3fv(locations.normalMatrixId, 1, GL_FALSE, glm::value_ptr(normalMatrix));
  for (auto& mesh : meshes) {
    glUniformMatrix4fv(locations.modelMatrixId, 1, GL_FALSE, glm::value_ptr(model));
    mesh->render(model);
  }
}