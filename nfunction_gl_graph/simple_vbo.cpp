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
#include <cmath>
#include <limits>

#include "simple_vbo.h"

namespace {

// Two-sided Moller-Trumbore ray/triangle intersection
bool rayIntersectsTriangle(const glm::vec3 &origin, const glm::vec3 &direction,
      const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, float &t) {
  glm::vec3 edge1 = v1 - v0;
  glm::vec3 edge2 = v2 - v0;
  glm::vec3 p = glm::cross(direction, edge2);
  float det = glm::dot(edge1, p);
  if (std::fabs(det) < 1e-12f) {
    return false;
  }
  float invDet = 1.0f / det;
  glm::vec3 s = origin - v0;
  float u = glm::dot(s, p) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return false;
  }
  glm::vec3 q = glm::cross(s, edge1);
  float v = glm::dot(direction, q) * invDet;
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }
  t = glm::dot(edge2, q) * invDet;
  return t > 0.0f;
}

bool rayIntersectsBox(const glm::vec3 &origin, const glm::vec3 &direction,
      const glm::vec3 &boxMin, const glm::vec3 &boxMax) {
  float tMin = 0.0f;
  float tMax = std::numeric_limits<float>::max();
  for (int axis = 0; axis < 3; axis++) {
    float invD = 1.0f / direction[axis];
    float t0 = (boxMin[axis] - origin[axis]) * invD;
    float t1 = (boxMax[axis] - origin[axis]) * invD;
    if (invD < 0.0f) {
      std::swap(t0, t1);
    }
    tMin = t0 > tMin ? t0 : tMin;
    tMax = t1 < tMax ? t1 : tMax;
    if (tMax < tMin) {
      return false;
    }
  }
  return true;
}

}

SimpleVbo::SimpleVbo(ShaderVarLocation locations, GLuint drawType) {
  vaoId = 0;
  positionNormalVbo = 0;
  colorVbo = 0;
  elementBufferObject = 0;
  indexCount = 0;
  verticeCount = 0;
  this->locations = locations;
  this->drawType = drawType;
  this->isReadyForRendering = false;
  this->position = glm::vec3(0.0f);
  this->rotationDegrees = glm::vec3(0.0f);
  this->scale = glm::vec3(1.0f);
  this->positionOffset = 0;
  this->normalOffset = -1;
  this->colorOffset = 0;
  this->strideInBytes = 3 * sizeof(float);
  this->colorStrideInBytes = 4 * sizeof(float);
  this->boundsMin = glm::vec3(0.0f);
  this->boundsMax = glm::vec3(0.0f);

  this->selected = false;
  this->frameVaoId = 0;
  this->frameVboId = 0;
  this->frameColorVboId = 0;
  this->frameElementBufferObject = 0;
}

/**
 * @brief Initialize the VBO object
 * 
 * @param vertices 
 * @param dataSize the number of floats in the vertices array
 * @param colors 
 * @param _verticeCount 
 * @param triangleTripIndices 
 * @param _indexCount 
 * @param _hasNormal 
 */
void SimpleVbo::initialize(const float* vertices, unsigned int dataSize, unsigned int _vertexCount,
              const float* colors, const unsigned short* triangleTripIndices, unsigned int _indexCount, 
                          int _normalOffset) {
  this->isReadyForRendering = false;
  this->normalOffset = _normalOffset;
  this->verticeCount = _vertexCount;
  this->indexCount = _indexCount;
  this->strideInBytes = normalOffset >= 0 ? 6 * sizeof(float) : 3 * sizeof(float);

  unsigned int floatsPerVertex = this->strideInBytes / sizeof(float);
  cpuPositions.clear();
  cpuPositions.reserve(_vertexCount * 3);
  for (unsigned int i = 0; i < _vertexCount && (i * floatsPerVertex + 2) < dataSize; i++) {
    cpuPositions.push_back(vertices[i * floatsPerVertex]);
    cpuPositions.push_back(vertices[i * floatsPerVertex + 1]);
    cpuPositions.push_back(vertices[i * floatsPerVertex + 2]);
  }
  cpuIndices.assign(triangleTripIndices, triangleTripIndices + (triangleTripIndices != nullptr ? _indexCount : 0));

  glGenVertexArrays(1, &vaoId);
  glBindVertexArray(vaoId);

  glGenBuffers(1, &positionNormalVbo);
  glBindBuffer(GL_ARRAY_BUFFER, positionNormalVbo);
  glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(float), vertices, GL_STATIC_DRAW);

  // Create a VBO for the colors
  if (colors != nullptr) {
    this->colorStrideInBytes = 4 * sizeof(float);
    glGenBuffers(1, &colorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, verticeCount * this->colorStrideInBytes, colors, GL_STATIC_DRAW);
  }

  // Create a buffer for the indices
  glGenBuffers(1, &elementBufferObject);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned short), triangleTripIndices, GL_STATIC_DRAW);

  this->buildCuboidFrame();
}

void SimpleVbo::setColor(const glm::vec4 &color) {
  this->isReadyForRendering = false;

  if (this->colorVbo != 0) {
    glBindBuffer(GL_ARRAY_BUFFER, this->colorVbo);
    float* colors = new float[this->verticeCount * 4];
    for (int i = 0; i < this->verticeCount; i++) {
      colors[i * 4] = color.r;
      colors[i * 4 + 1] = color.g;
      colors[i * 4 + 2] = color.b;
      colors[i * 4 + 3] = color.a;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, this->verticeCount * this->colorStrideInBytes, colors);
    delete[] colors;
  } else {
    this->colorStrideInBytes = 4 * sizeof(float);
    float* colors = new float[this->verticeCount * 4];
    for (int i = 0; i < this->verticeCount; i++) {
      colors[i * 4] = color.r;
      colors[i * 4 + 1] = color.g;
      colors[i * 4 + 2] = color.b;
      colors[i * 4 + 3] = color.a;
    }
    glGenBuffers(1, &colorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->colorVbo);
    glBufferData(GL_ARRAY_BUFFER, verticeCount * this->colorStrideInBytes, colors, GL_STATIC_DRAW);
  }

  this->isReadyForRendering = true;
}

/**
 * @brief Build a cuboid frame, that wraps around this VBO object.
 * Also computes the local-space bounding box used for picking.
 */
void SimpleVbo::buildCuboidFrame() {
  if (cpuPositions.empty()) {
    return;
  }
  float minX = cpuPositions[0];
  float maxX = cpuPositions[0];
  float minY = cpuPositions[1];
  float maxY = cpuPositions[1];
  float minZ = cpuPositions[2];
  float maxZ = cpuPositions[2];
  for (size_t k = 0; k + 2 < cpuPositions.size(); k += 3) {
    if (cpuPositions[k] < minX) minX = cpuPositions[k];
    if (cpuPositions[k] > maxX) maxX = cpuPositions[k];
    if (cpuPositions[k + 1] < minY) minY = cpuPositions[k + 1];
    if (cpuPositions[k + 1] > maxY) maxY = cpuPositions[k + 1];
    if (cpuPositions[k + 2] < minZ) minZ = cpuPositions[k + 2];
    if (cpuPositions[k + 2] > maxZ) maxZ = cpuPositions[k + 2];
  }
  boundsMin = glm::vec3(minX, minY, minZ);
  boundsMax = glm::vec3(maxX, maxY, maxZ);

  float cuboidVertices[] = {
    minX, minY, minZ,
    maxX, minY, minZ,
    maxX, maxY, minZ,
    minX, maxY, minZ,
    minX, minY, maxZ,
    maxX, minY, maxZ,
    maxX, maxY, maxZ,
    minX, maxY, maxZ
  };
  float cuboidColors[] = {
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f
  };
  GLuint cuboidDrawType = GL_LINES;
  unsigned short cuboidIndices[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
  };

  glGenVertexArrays(1, &frameVaoId);
  glBindVertexArray(frameVaoId);

  glGenBuffers(1, &frameVboId);
  glBindBuffer(GL_ARRAY_BUFFER, frameVboId);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), cuboidVertices, GL_STATIC_DRAW);

  glGenBuffers(1, &frameColorVboId);
  glBindBuffer(GL_ARRAY_BUFFER, frameColorVboId);
  glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(float), cuboidColors, GL_STATIC_DRAW);
  
  // Create a buffer for the indices
  glGenBuffers(1, &frameElementBufferObject);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frameElementBufferObject);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(unsigned short), cuboidIndices, GL_STATIC_DRAW);
}

void SimpleVbo::renderFrame(const glm::mat4 &combinedModel) {
  if (this->frameVaoId == 0) {
    std::cout << "Frame VBO is not initialized" << std::endl;
    return;
  }
  
  glUniformMatrix4fv(locations.modelMatrixId, 1, GL_FALSE, glm::value_ptr(combinedModel));
  glUniform1i(locations.useLightingLocation, 0);
  glUniform1i(locations.selectedLocation, 0);

  glBindVertexArray(this->frameVaoId);
  glBindBuffer(GL_ARRAY_BUFFER, this->frameVboId);
  glEnableVertexAttribArray(this->locations.positionLocation);
  GLsizeiptr frameStride = 3 * sizeof(float);
  glVertexAttribPointer(this->locations.positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        frameStride,
                        reinterpret_cast<void*>(0));
  
  glBindBuffer(GL_ARRAY_BUFFER, this->frameColorVboId);
  glEnableVertexAttribArray(this->locations.colorLocation);
  frameStride = 4 * sizeof(float);
  glVertexAttribPointer(this->locations.colorLocation, //Attribute index
                        4,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        frameStride,
                        reinterpret_cast<void*>(0));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->frameElementBufferObject);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void SimpleVbo::setupArrayAttributes() {
  if (this->vaoId == 0) {
#ifdef _DEBUG_
    std::cout << "VBO is not initialized" << std::endl;
#endif
    return;
  }
  if (this->positionNormalVbo == 0) {
#ifdef _DEBUG_
    std::cout << "Position/Normal VBO is not initialized" << std::endl;
#endif
    return;
  }
  if (this->colorVbo == 0) {
#ifdef _DEBUG_
    std::cout << "Color VBO is not initialized" << std::endl;
#endif
    return;
  }
  
  glBindVertexArray(vaoId);
  // Set up position attribute (location 0)
  glBindBuffer(GL_ARRAY_BUFFER, positionNormalVbo);
  glVertexAttribPointer(locations.positionLocation, 3, GL_FLOAT, GL_FALSE, this->strideInBytes, (void*)0);
  glEnableVertexAttribArray(locations.positionLocation);

  // Set up normal attribute (location 1)
  if (this->normalOffset >= 0) {
    glVertexAttribPointer(locations.normalLocation, 3, GL_FLOAT, GL_FALSE, this->strideInBytes, (void*)(this->normalOffset * sizeof(float)));  // Offset by 3 floats
    glEnableVertexAttribArray(locations.normalLocation);
  }

  // Set up color attribute (location 2)
  glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
  glVertexAttribPointer(locations.colorLocation, 4, GL_FLOAT, GL_FALSE, this->colorStrideInBytes, (void*)0);
  glEnableVertexAttribArray(locations.colorLocation);

  // Bind the element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);

  // Unbind VAO first, then buffers
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  this->isReadyForRendering = true;
}

void SimpleVbo::applyRotation(float rad, const glm::vec3 &axis) {
  glm::quat current = glm::quat(glm::radians(this->rotationDegrees));
  glm::quat rotation = glm::angleAxis(rad, glm::normalize(axis));
  // Accumulate the rotation
  this->rotationDegrees = glm::degrees(glm::eulerAngles(rotation * current));
}

void SimpleVbo::applyTranslation(const glm::vec3 &translation) {
  this->position += translation;
}

void SimpleVbo::applyScale(const glm::vec3 &scale) {
  setScale(this->scale * scale);
}

void SimpleVbo::resetTransform() {
  this->position = glm::vec3(0.0f);
  this->rotationDegrees = glm::vec3(0.0f);
  this->scale = glm::vec3(1.0f);
}

glm::mat4 SimpleVbo::getModelMatrix() const {
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), this->position);
  glm::mat4 rotation = glm::mat4_cast(glm::quat(glm::radians(this->rotationDegrees)));
  glm::mat4 scaling = glm::scale(glm::mat4(1.0f), this->scale);
  return translation * rotation * scaling;
}

bool SimpleVbo::intersectRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection,
        const glm::mat4 &globalModel, float &hitDistance) const {
  if (cpuPositions.empty()) {
    return false;
  }
  if (drawType != GL_TRIANGLES && drawType != GL_TRIANGLE_STRIP && drawType != GL_TRIANGLE_FAN) {
    return false;
  }

  // Bring the ray into the object's local space so the stored vertices can be used as-is
  glm::mat4 combinedModel = globalModel * getModelMatrix();
  glm::mat4 inverseModel = glm::inverse(combinedModel);
  glm::vec3 localOrigin = glm::vec3(inverseModel * glm::vec4(rayOrigin, 1.0f));
  glm::vec3 localDirection = glm::vec3(inverseModel * glm::vec4(rayDirection, 0.0f));

  if (!rayIntersectsBox(localOrigin, localDirection, boundsMin, boundsMax)) {
    return false;
  }

  unsigned int localVertexCount = cpuPositions.size() / 3;
  unsigned int elementCount = cpuIndices.empty() ? localVertexCount : cpuIndices.size();
  auto vertexAt = [&](unsigned int element) -> unsigned int {
    return cpuIndices.empty() ? element : cpuIndices[element];
  };
  auto position = [&](unsigned int idx) {
    return glm::vec3(cpuPositions[idx * 3], cpuPositions[idx * 3 + 1], cpuPositions[idx * 3 + 2]);
  };

  bool hit = false;
  float closestT = std::numeric_limits<float>::max();
  auto testTriangle = [&](unsigned int e0, unsigned int e1, unsigned int e2) {
    unsigned int i0 = vertexAt(e0);
    unsigned int i1 = vertexAt(e1);
    unsigned int i2 = vertexAt(e2);
    if (i0 >= localVertexCount || i1 >= localVertexCount || i2 >= localVertexCount) {
      return;
    }
    float t;
    if (rayIntersectsTriangle(localOrigin, localDirection, position(i0), position(i1), position(i2), t)
        && t < closestT) {
      closestT = t;
      hit = true;
    }
  };

  if (drawType == GL_TRIANGLES) {
    for (unsigned int e = 0; e + 2 < elementCount; e += 3) {
      testTriangle(e, e + 1, e + 2);
    }
  } else if (drawType == GL_TRIANGLE_STRIP) {
    for (unsigned int e = 0; e + 2 < elementCount; e++) {
      testTriangle(e, e + 1, e + 2);
    }
  } else {
    for (unsigned int e = 1; e + 1 < elementCount; e++) {
      testTriangle(0, e, e + 1);
    }
  }

  if (!hit) {
    return false;
  }

  glm::vec3 worldHit = glm::vec3(combinedModel * glm::vec4(localOrigin + localDirection * closestT, 1.0f));
  hitDistance = glm::length(worldHit - rayOrigin);
  return true;
}

void SimpleVbo::render(const glm::mat4 &globalModel) {
  if (!this->isReadyForRendering) {
#ifdef _DEBUG_
    std::cout << "VBO is not ready for rendering" << std::endl;
#endif
    return;
  }

  glm::mat4 combinedModel = globalModel * getModelMatrix();

  if (this->selected) {
    renderFrame(combinedModel);
  }

  glUniformMatrix4fv(locations.modelMatrixId, 1, GL_FALSE, glm::value_ptr(combinedModel));
  glUniform1i(locations.selectedLocation, this->selected ? 1 : 0);

  // Turn on lighting
  glUniform1i(locations.useLightingLocation, 1);
  glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(combinedModel)));
  glUniformMatrix3fv(locations.normalMatrixId, 1, GL_FALSE, glm::value_ptr(normalMatrix));

  glBindVertexArray(this->vaoId);
  if (this->indexCount > 0) {
    // std::cout << "Drawing VBO with indices" << std::endl;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementBufferObject);
    glDrawElements(this->drawType, this->indexCount, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
  } else {
    // std::cout << "Drawing VBO without indices" << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, this->positionNormalVbo);
    glEnableVertexAttribArray(this->locations.positionLocation);
    glVertexAttribPointer(this->locations.positionLocation, //Attribute index
                        3,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        this->strideInBytes,
                        reinterpret_cast<void*>(0));
    if (this->normalOffset >= 0) {
      glEnableVertexAttribArray(this->locations.normalLocation);
      glVertexAttribPointer(this->locations.normalLocation, //Attribute index
                          3,  //Number of component per this attribute of vertex
                          GL_FLOAT,
                          GL_FALSE,
                          this->strideInBytes,
                          reinterpret_cast<void*>(this->normalOffset));
    }
    if (this->colorVbo != 0) {
      glBindBuffer(GL_ARRAY_BUFFER, this->colorVbo);
      glEnableVertexAttribArray(this->locations.colorLocation);
      glVertexAttribPointer(this->locations.colorLocation, //Attribute index
                        4,  //Number of component per this attribute of vertex
                        GL_FLOAT,
                        GL_FALSE,
                        this->colorStrideInBytes,
                        reinterpret_cast<void*>(this->colorOffset));
    }
    glDrawArrays(this->drawType, 0, this->verticeCount);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisableVertexAttribArray(this->locations.positionLocation);
    glDisableVertexAttribArray(this->locations.normalLocation);
    glDisableVertexAttribArray(this->locations.colorLocation);
  }
  glUniform1i(locations.selectedLocation, 0);
}

SimpleVbo::~SimpleVbo() {
  release();
}

void SimpleVbo::release() {
  glDeleteVertexArrays(1, &vaoId);
  glDeleteBuffers(1, &positionNormalVbo);
  glDeleteBuffers(1, &colorVbo);
  glDeleteBuffers(1, &elementBufferObject);
  vaoId = 0;
  positionNormalVbo = 0;
  colorVbo = 0;
  elementBufferObject = 0;

  glDeleteVertexArrays(1, &frameVaoId);
  glDeleteBuffers(1, &frameVboId);
  glDeleteBuffers(1, &frameColorVboId);
  glDeleteBuffers(1, &frameElementBufferObject);
  frameVaoId = 0;
  frameVboId = 0;
  frameColorVboId = 0;
  frameElementBufferObject = 0;
  this->isReadyForRendering = false;
  this->selected = false;
  resetTransform();
  cpuPositions.clear();
  cpuIndices.clear();
  this->positionOffset = 0;
  this->normalOffset = -1;
  this->colorOffset = 0;
  this->strideInBytes = 3 * sizeof(float);
  this->colorStrideInBytes = 4 * sizeof(float);
#ifdef _DEBUG_
  std::cout << "Done releasing VBO" << std::endl;
#endif
}