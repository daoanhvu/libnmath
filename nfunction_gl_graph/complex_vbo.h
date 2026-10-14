#ifndef _COMPLEX_VBO_H_
#define _COMPLEX_VBO_H_

#include <vector>
#include <memory>
#include "vbo.h"

class ComplexVbo: public VBO {
  private:
    std::vector<std::unique_ptr<VBO>> components;

  public:

    void initialize(const float* vertices, 
      unsigned int dataSize, 
      unsigned int _vertexCount,
      const float* colors, 
      const unsigned short* triangleTripIndices, 
      unsigned int _indexCount, 
      int _normalOffset) override {}

    ~ComplexVbo() override;

    void release() override {}

    void render(const glm::mat4 &globalModel) override {}

    void resetTransform() override {}

    bool intersectRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection,
      const glm::mat4 &globalModel, float &hitDistance) const override { return false; }

    int countComponents() const {
      return components.size();
    }
};

#endif