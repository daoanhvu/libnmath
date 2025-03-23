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

#include "vbo_objects.h"

VboObject::VboObject(ShaderVarLocation locations, GLuint drawType) {
  vaoId = 0;
  positionNormalVbo = 0;
  colorVbo = 0;
  elementBufferObject = 0;
  indexCount = 0;
  colorOffset = 0;
  normalOffset = -1;
  colorStrideInBytes = 4 * sizeof(float);
  this->locations = locations;
  this->drawType = drawType;
  this->isReadyForRendering = false;
}

/**
 * @brief Initialize the VBO object
 * 
 * @param vertices 
 * @param colors 
 * @param _verticeCount 
 * @param triangleTripIndices 
 * @param _indexCount 
 * @param _hasNormal 
 */
void VboObject::initialize(const float* vertices, unsigned int dataSize, unsigned int _vertexCount,
              const float* colors, const unsigned short* triangleTripIndices, unsigned int _indexCount, 
                          unsigned int _normalOffset) {
  this->isReadyForRendering = false;
  this->normalOffset = _normalOffset;
  this->verticeCount = _vertexCount;
  this->indexCount = _indexCount;
  this->strideInBytes = normalOffset >= 0 ? 6 * sizeof(float) : 3 * sizeof(float);

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
}

void VboObject::setupArrayAttributes() {
  glGenVertexArrays(1, &vaoId);
  glBindVertexArray(vaoId);

  // Set up position attribute (location 0)
  glBindBuffer(GL_ARRAY_BUFFER, positionNormalVbo);
  glVertexAttribPointer(locations.positionLocation, 3, GL_FLOAT, GL_FALSE, this->strideInBytes, (void*)0);
  glEnableVertexAttribArray(locations.positionLocation);

  // Set up normal attribute (location 1)
  glVertexAttribPointer(locations.normalLocation, 3, GL_FLOAT, GL_FALSE, this->strideInBytes, (void*)(this->normalOffset * sizeof(float)));  // Offset by 3 floats
  glEnableVertexAttribArray(locations.normalLocation);

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

void VboObject::applyRotation(float xRadian, const glm::vec3 &xAxis, float yRadian, const glm::vec3 &yAxis, 
                                float zRadian, const glm::vec3 &zAxis) {

}

void VboObject::render() {
  if (!this->isReadyForRendering) {
    std::cout << "VBO is not ready for rendering" << std::endl;
    return;
  }

  if (this->indexCount > 0) {
    // std::cout << "Drawing VBO with indices" << std::endl;
    glBindVertexArray(this->vaoId);
    glDrawElements(this->drawType, this->indexCount, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
  } else {
    // std::cout << "Drawing VBO without indices" << std::endl;
    glBindVertexArray(this->vaoId);
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
}

VboObject::~VboObject() {
  release();
}

void VboObject::release() {
  glDeleteVertexArrays(1, &vaoId);
  glDeleteBuffers(1, &positionNormalVbo);
  glDeleteBuffers(1, &colorVbo);
  glDeleteBuffers(1, &elementBufferObject);
  std::cout << "Done releasing VBO" << std::endl;
}