#ifndef _VBO_H_
#define _VBO_H_

#include <string>
#include <glm/glm.hpp>

class VBO {
  protected:
    bool selected;
    std::string name;

    glm::vec3 position;
    // Euler angles in degrees (pitch, yaw, roll)
    glm::vec3 rotationDegrees;
    glm::vec3 scale;
    bool isReadyForRendering;

  public:

    virtual void initialize(const float* vertices, unsigned int dataSize, unsigned int _vertexCount,
      const float* colors, const unsigned short* triangleTripIndices, unsigned int _indexCount, 
                int _normalOffset) = 0;

    virtual ~VBO() = default;
    virtual void release() = 0;

    virtual void resetTransform() = 0;

    virtual const glm::vec3 &getPosition() const { return position; }
    virtual void setPosition(const glm::vec3 &value) { position = value; }

    virtual const glm::vec3 &getRotationDegrees() const { return rotationDegrees; }
    virtual void setRotationDegrees(const glm::vec3 &value) { rotationDegrees = value; }

    virtual const glm::vec3 &getScale() const { return scale; }
    virtual void setScale(const glm::vec3 &value);

    virtual bool isSelected() const { return selected; }
    virtual void setSelected(bool value) { selected = value; }

    virtual const std::string &getName() const { return name; }
    virtual void setName(const std::string &value) { name = value; }

    virtual void render(const glm::mat4 &globalModel) = 0;

    virtual void setupArrayAttributes() = 0;
    virtual bool intersectRay(const glm::vec3 &rayOrigin, 
      const glm::vec3 &rayDirection,
      const glm::mat4 &globalModel, 
      float &hitDistance) const = 0;

};

#endif