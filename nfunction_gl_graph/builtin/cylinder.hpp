#ifndef __CYLINDER_HPP_
#define __CYLINDER_HPP_

#include <cmath>
#include <vector>
#include "../gl_type.h"

class Cylinder {
public:
  Cylinder(float h, float r, int st, int cst) : height(h), radius(r), stacks(st), capStacks(cst) {}

  float getVolume() const {
    return M_PI * radius * radius * height;
  }

  float getSurfaceArea() const {
    return 2.0f * M_PI * radius * (height + radius);
  }

  /**
   * Compute the position of a point on the cylinder wall using parametric equations
   *   x = radius * cos(u)
   *   y = radius * sin(u)
   *   z = v
   * Param u: angle around the cylinder (0 to 2*PI)
   * Param v: height along the cylinder (0 to height)
   */
  Point3 computeCylinderPosition(float u, float v) const {
    float x = radius * cos(u);
    float y = radius * sin(u);
    return Point3(x, y, v);
  }

  /**
   * Compute the normal vector at a point on the cylinder wall
   * Tangents:
   *   du = (-radius * sin(u), radius * cos(u), 0)
   *   dv = (0, 0, 1)
   * Outward normal is normalize(du × dv) = (cos(u), sin(u), 0)
   * Param u: angle around the cylinder (0 to 2*PI)
   * Param v: height along the cylinder (0 to height)
   */
  Point3 computeCylinderNormal(float u, float v) const {
    (void)v;
    Point3 du(-radius * sin(u), radius * cos(u), 0.0f);
    Point3 dv(0.0f, 0.0f, 1.0f);
    return (du.cross(dv)).normalize();
  }

  /**
   * Position on a flat cap. v is the radial fraction from the center to the rim.
   *   x = v * radius * cos(u)
   *   y = v * radius * sin(u)
   *   z = 0 on the bottom, z = height on the top
   * Param u: angle around the cap (0 to 2*PI)
   * Param v: radial fraction (0 at the center, 1 at the rim)
   */
  Point3 computeCapPosition(float u, float v, bool top) const {
    float radial = v * radius;
    float x = radial * cos(u);
    float y = radial * sin(u);
    float z = top ? height : 0.0f;
    return Point3(x, y, z);
  }

  /**
   * Flat caps have a constant normal: +Z on the top, -Z on the bottom
   */
  Point3 computeCapNormal(bool top) const {
    return top ? Point3(0.0f, 0.0f, 1.0f) : Point3(0.0f, 0.0f, -1.0f);
  }

  /**
   * Generate the mesh data for the cylinder wall and both flat caps
   * Param vertices: output vector to store vertex data (position, then normal)
   * Param indices: output vector to store index data
   * Return: number of vertices generated
   */
  int generateMesh(std::vector<float> &vertices, std::vector<unsigned short> &indices) const {
    int slices = 24;

    for (int i = 0; i <= stacks; ++i) {
      float v = height * static_cast<float>(i) / static_cast<float>(stacks);
      for (int j = 0; j <= slices; ++j) {
        float u = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
        Point3 pos = computeCylinderPosition(u, v);
        Point3 normal = computeCylinderNormal(u, v);
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
      }
    }

    for (int i = 0; i < stacks; ++i) {
      for (int j = 0; j < slices; ++j) {
        unsigned short v0 = (i * (slices + 1)) + j;
        unsigned short v1 = v0 + 1;
        unsigned short v2 = (i + 1) * (slices + 1) + j;
        unsigned short v3 = v2 + 1;
        indices.push_back(v0);
        indices.push_back(v1);
        indices.push_back(v2);
        indices.push_back(v1);
        indices.push_back(v3);
        indices.push_back(v2);
      }
    }

    appendCap(vertices, indices, slices, false);
    appendCap(vertices, indices, slices, true);

    return vertices.size() / 6;
  }

private:
  float radius;
  float height;
  int stacks;
  int capStacks;

  void pushVertex(std::vector<float> &vertices, const Point3 &pos, const Point3 &normal) const {
    vertices.push_back(pos.x);
    vertices.push_back(pos.y);
    vertices.push_back(pos.z);
    vertices.push_back(normal.x);
    vertices.push_back(normal.y);
    vertices.push_back(normal.z);
  }

  /**
   * Disk centered on the axis. Ring 0 is the center, ring capStacks is the rim.
   * Top winding faces +Z. Bottom winding faces -Z.
   */
  void appendCap(std::vector<float> &vertices, std::vector<unsigned short> &indices, int slices, bool top) const {
    unsigned int capStart = vertices.size() / 6;
    Point3 normal = computeCapNormal(top);

    for (int i = 0; i <= capStacks; ++i) {
      float v = static_cast<float>(i) / static_cast<float>(capStacks);
      for (int j = 0; j <= slices; ++j) {
        float u = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
        pushVertex(vertices, computeCapPosition(u, v, top), normal);
      }
    }

    for (int i = 0; i < capStacks; ++i) {
      for (int j = 0; j < slices; ++j) {
        unsigned short v0 = capStart + (i * (slices + 1)) + j;
        unsigned short v1 = v0 + 1;
        unsigned short v2 = capStart + ((i + 1) * (slices + 1)) + j;
        unsigned short v3 = v2 + 1;
        if (top) {
          indices.push_back(v0);
          indices.push_back(v2);
          indices.push_back(v1);
          indices.push_back(v1);
          indices.push_back(v2);
          indices.push_back(v3);
        } else {
          indices.push_back(v0);
          indices.push_back(v1);
          indices.push_back(v2);
          indices.push_back(v1);
          indices.push_back(v3);
          indices.push_back(v2);
        }
      }
    }
  }
};

#endif // __CYLINDER_HPP_
