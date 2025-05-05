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
  this->locations = locations;
  this->drawType = drawType;
  this->isReadyForRendering = false;
  this->rotationMatrix = glm::mat4(1.0f);
  this->translationMatrix = glm::mat4(1.0f);
  this->scaleMatrix = glm::mat4(1.0f);
  this->positionOffset = 0;
  this->normalOffset = -1;
  this->colorOffset = 0;
  this->strideInBytes = 3 * sizeof(float);
  this->colorStrideInBytes = 4 * sizeof(float);

  this->isSelected = false;
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
void VboObject::initialize(const float* vertices, unsigned int dataSize, unsigned int _vertexCount,
              const float* colors, const unsigned short* triangleTripIndices, unsigned int _indexCount, 
                          unsigned int _normalOffset) {
  this->isReadyForRendering = false;
  this->normalOffset = _normalOffset;
  this->verticeCount = _vertexCount;
  this->indexCount = _indexCount;
  this->strideInBytes = normalOffset >= 0 ? 6 * sizeof(float) : 3 * sizeof(float);

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

  this->buildCuboidFrame(vertices, dataSize, _vertexCount);
}

void VboObject::setColor(const glm::vec4 &color) {
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
 * @brief Build a cuboid frame, that wraps around this VBO object
 * 
 * @param vertices 
 * @param dataSize 
 * @param _vertexCount 
 */
void VboObject::buildCuboidFrame(const float* vertices, unsigned int dataSize, unsigned int _vertexCount) {
  float minX = vertices[0];
  float maxX = vertices[0];
  float minY = vertices[1];
  float maxY = vertices[1];
  float minZ = vertices[2];
  float maxZ = vertices[2];
  unsigned int k;
  for (unsigned int i = 0; i < _vertexCount; i++) {
    k = i * 3;
    if (vertices[k] < minX) minX = vertices[k];
    if (vertices[k] > maxX) maxX = vertices[k];
    if (vertices[k + 1] < minY) minY = vertices[k + 1];
    if (vertices[k + 1] > maxY) maxY = vertices[k + 1];
    if (vertices[k + 2] < minZ) minZ = vertices[k + 2];
    if (vertices[k + 2] > maxZ) maxZ = vertices[k + 2];
  }
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

void VboObject::renderFrame(const glm::mat4 &combinedModel) {
  if (this->frameVaoId == 0) {
    std::cout << "Frame VBO is not initialized" << std::endl;
    return;
  }
  
  glUniformMatrix4fv(locations.modelMatrixId, 1, GL_FALSE, glm::value_ptr(combinedModel));
  glUniform1i(locations.useLightingLocation, 0);

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

void VboObject::setupArrayAttributes() {
  if (this->vaoId == 0) {
    std::cout << "VBO is not initialized" << std::endl;
    return;
  }
  if (this->positionNormalVbo == 0) {
    std::cout << "Position/Normal VBO is not initialized" << std::endl;
    return;
  }
  if (this->colorVbo == 0) {
    std::cout << "Color VBO is not initialized" << std::endl;
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

void VboObject::applyRotation(float rad, const glm::vec3 &axis) {
  glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rad, axis);
  // Accumulate the rotation
  this->rotationMatrix = rotation * this->rotationMatrix;
}

void VboObject::applyTranslation(const glm::vec3 &translation) {
  this->translationMatrix = glm::translate(this->translationMatrix, translation);
}

void VboObject::applyScale(const glm::vec3 &scale) {
  this->scaleMatrix = glm::scale(this->scaleMatrix, scale);
}

void VboObject::render(const glm::mat4 &globalModel) {
  if (!this->isReadyForRendering) {
    std::cout << "VBO is not ready for rendering" << std::endl;
    return;
  }

  glm::mat4 modelMatrix = this->translationMatrix * this->rotationMatrix * this->scaleMatrix;
  glm::mat4 combinedModel = globalModel * modelMatrix;
  glUniformMatrix4fv(locations.modelMatrixId, 1, GL_FALSE, glm::value_ptr(combinedModel));

  if (this->isSelected) {
    renderFrame(combinedModel);
  }

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
}

VboObject::~VboObject() {
  release();
}

void VboObject::release() {
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
  this->isSelected = false;
  this->rotationMatrix = glm::mat4(1.0f);
  this->translationMatrix = glm::mat4(1.0f);
  this->scaleMatrix = glm::mat4(1.0f);
  this->positionOffset = 0;
  this->normalOffset = -1;
  this->colorOffset = 0;
  this->strideInBytes = 3 * sizeof(float);
  this->colorStrideInBytes = 4 * sizeof(float);
  std::cout << "Done releasing VBO" << std::endl;
}