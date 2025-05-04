#ifndef _VBO_H_
#define _VBO_H_

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "shaders_utils.h"

class VboObject {
  private:
    GLuint vaoId;
    // position, color bufferId
    GLuint positionNormalVbo;
    GLuint colorVbo;
    GLuint elementBufferObject;
    unsigned int indexCount;
    // This is THE number of vertice in the VBO, NOT the number of float in [vertices]
    int verticeCount;
    GLsizeiptr strideInBytes;
    GLintptr positionOffset;
    GLintptr normalOffset;
    GLintptr colorOffset;
    GLintptr colorStrideInBytes;
    // shader's variables locations
    ShaderVarLocation locations;
    glm::mat4 rotationMatrix;
    glm::mat4 translationMatrix;
    glm::mat4 scaleMatrix;
    GLuint drawType;
    bool isReadyForRendering;

    bool isSelected;

    GLuint frameVaoId;
    GLuint frameVboId;
    GLuint frameColorVboId;
    GLuint frameElementBufferObject;

    public:
      VboObject(ShaderVarLocation locations, GLuint drawType);

      void initialize(const float* vertices, unsigned int dataSize, unsigned int _vertexCount,
              const float* colors, const unsigned short* triangleTripIndices, unsigned int _indexCount, 
                          unsigned int _normalOffset);

      void buildCuboidFrame(const float* vertices, unsigned int dataSize, unsigned int _vertexCount);

      void setupArrayAttributes();

      virtual ~VboObject();

      void release();

      void applyRotation(float rad, const glm::vec3 &axis);

      void applyTranslation(const glm::vec3 &translation);

      void applyScale(const glm::vec3 &scale);

      void render(const glm::mat4 &globalModel);

      void renderFrame(const glm::mat4 &globalModel);
};

#endif