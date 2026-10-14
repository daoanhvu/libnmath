#include "vbo.h"

const float MIN_SCALE = 0.001f;

void VBO::setScale(const glm::vec3 &value) {
  // A zero scale would make the model matrix non-invertible, which breaks picking and normals
  this->scale = glm::max(value, glm::vec3(MIN_SCALE));
}