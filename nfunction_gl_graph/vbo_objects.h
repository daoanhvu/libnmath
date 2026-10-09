#ifndef _VBO_H_
#define _VBO_H_

#include <iostream>
#include <string>
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
    // Object transform, applied as translate * rotate * scale
    glm::vec3 position;
    // Euler angles in degrees (pitch, yaw, roll)
    glm::vec3 rotationDegrees;
    glm::vec3 scale;
    GLuint drawType;
    bool isReadyForRendering;

    bool selected;
    std::string name;

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
      VboObject(ShaderVarLocation locations, GLuint drawType);

      void initialize(const float* vertices, unsigned int dataSize, unsigned int _vertexCount,
              const float* colors, const unsigned short* triangleTripIndices, unsigned int _indexCount, 
                          int _normalOffset);

      void buildCuboidFrame();

      void setupArrayAttributes();

      void setColor(const glm::vec4 &color);

      virtual ~VboObject();

      void release();

      void applyRotation(float rad, const glm::vec3 &axis);

      void applyTranslation(const glm::vec3 &translation);

      void applyScale(const glm::vec3 &scale);

      const glm::vec3 &getPosition() const { return position; }
      void setPosition(const glm::vec3 &value) { position = value; }

      const glm::vec3 &getRotationDegrees() const { return rotationDegrees; }
      void setRotationDegrees(const glm::vec3 &value) { rotationDegrees = value; }

      const glm::vec3 &getScale() const { return scale; }
      void setScale(const glm::vec3 &value);

      void resetTransform();

      glm::mat4 getModelMatrix() const;

      bool isSelected() const { return selected; }
      void setSelected(bool value) { selected = value; }

      const std::string &getName() const { return name; }
      void setName(const std::string &value) { name = value; }

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
              const glm::mat4 &globalModel, float &hitDistance) const;

      void render(const glm::mat4 &globalModel);

      void renderFrame(const glm::mat4 &globalModel);
};

#endif
