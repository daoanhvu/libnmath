#ifndef _SIMPLE_VBO_H_
#define _SIMPLE_VBO_H_

#include <iostream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "vbo.h"
#include "shaders_utils.h"

class SimpleVbo: public VBO {
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
    GLuint drawType;

    // CPU copies of the geometry (local space), used for mouse picking
    std::vector<float> cpuPositions;
    std::vector<unsigned short> cpuIndices;
    glm::vec3 boundsMin;
    glm::vec3 boundsMax;

    GLuint frameVaoId;
    GLuint frameVboId;
    GLuint frameColorVboId;
    GLuint frameElementBufferObject;

  public:
      SimpleVbo(ShaderVarLocation locations, GLuint drawType);

      void initialize(const float* vertices, unsigned int dataSize, unsigned int _vertexCount,
              const float* colors, const unsigned short* triangleTripIndices, unsigned int _indexCount, 
                          int _normalOffset);

      void buildCuboidFrame();

      void setupArrayAttributes() override ;

      void setColor(const glm::vec4 &color);

      virtual ~SimpleVbo() override ;

      void release() override;

      void applyRotation(float rad, const glm::vec3 &axis);

      void applyTranslation(const glm::vec3 &translation);

      void applyScale(const glm::vec3 &scale);

      void resetTransform() override;

      glm::mat4 getModelMatrix() const;

      /**
       * @brief Test a world-space ray against this object's triangles
       *
       * @param rayOrigin world-space ray origin
       * @param rayDirection world-space ray direction
       * @param globalModel the scene transform this object is rendered with
       * @param hitDistance world-space distance from rayOrigin to the closest hit
       * @return true if the ray hits the object
       */
      bool intersectRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection,
              const glm::mat4 &globalModel, float &hitDistance) const override;

      void render(const glm::mat4 &globalModel) override;

      void renderFrame(const glm::mat4 &globalModel);
};

#endif
