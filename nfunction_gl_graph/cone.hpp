#include <cmath>
#include <vector>

struct Point3 {
  float x;
  float y;
  float z;

  Point3() : x(0.0f), y(0.0f), z(0.0f) {}
  Point3(float x, float y, float z) : x(x), y(y), z(z) {}

  Point3 operator+(const Point3& other) const {
    return Point3(x + other.x, y + other.y, z + other.z);
  }

  Point3 operator-(const Point3& other) const {
    return Point3(x - other.x, y - other.y, z - other.z);
  }

  Point3 operator*(float scalar) const {
    return Point3(x * scalar, y * scalar, z * scalar);
  }

  Point3 cross(const Point3& other) const {
    return Point3(
      y * other.z - z * other.y,
      z * other.x - x * other.z,
      x * other.y - y * other.x
    );
  }

  Point3 normalize() const {
    float length = sqrt(x * x + y * y + z * z);
    return Point3(x / length, y / length, z / length);
  }
};

class Cone {
public:
  Cone(float radius, float height) : radius(radius), height(height) {}

  float getVolume() const {
      return (1.0f / 3.0f) * M_PI * radius * radius * height;
  }

  float getSurfaceArea() const {
      return M_PI * radius * (radius + sqrt(height * height + radius * radius));
  }

  /**
   * Compute the position of a point on the cone surface using parametric equations
   * Param u: angle around the cone (0 to 2*PI)
   * Param v: height along the cone (0 to height)
   */
  Point3 computeConePosition(float u, float v) const {
    float z = v * (height - capRadius);
    // Compute the radius of the cone at height z
    float coneRadius = radius * (height - z) / height;
    float x = coneRadius * cos(u);
    float y = coneRadius * sin(u);
    return Point3(x, y, z);
  }

  /**
   * Compute the normal vector at a point on the cone surface
   * Parametric: 
   *    x = radius * (height - z) / height * cos(u)
   *    y = radius * (height - z) / height * sin(u)
   *    z = v * (height - capRadius)
   * Tangents: du, dv 
   * Param u: angle around the cone (0 to 2*PI)
   * Param v: height along the cone (0 to height)
   */
  Point3 computeConeNormal(float u, float v) const {
    float z = v * (height - capRadius);
    float coneRadius = radius * (height - z) / height;
    Point3 du(-coneRadius * sin(u), coneRadius * cos(u), 0.0f);
    Point3 dv(-radius / height * cos(u), -radius / height * sin(u), height - capRadius);
    return (dv.cross(du)).normalize();
  }

  /**
   * Generate the mesh data for the cap of the cone
   * Note: Spherical cap centered at (0, 0, height - capRadius)
   * theta from 0 to thetaMax, phi = u
   */
  Point3 computeCapPosition(float u, float v) const {
    float thetaMax = asin(capRadius / (height - capRadius));
    float theta = thetaMax * v;
    float x = capRadius * sin(theta) * cos(u);
    float y = capRadius * sin(theta) * sin(u);
    // z = height - capRadius + capRadius * cos(theta)
    float z = (height - capRadius) + capRadius * cos(theta);
    return Point3(x, y, z);
  }

  /**
   * Compute the normal vector at a point on the cap surface of the cone
   * Note: For sphere, normal is simply the normalized position vector relative to the center
   */
  Point3 computeCapNormal(float u, float v) const {
    Point3 pos = computeCapPosition(u, v);
    Point3 center = Point3(0.0f, 0.0f, height - capRadius);
    return (pos - center).normalize();
  }

  void generateMesh(std::vector<float> &vertices, std::vector<unsigned int> &indices) const {
    
    // slice is alzimuthal division of the cone
    int slices = 24;

    // compute the cone body vertices
    for (int i = 0; i <= stacks; ++i) {
      float v = height * static_cast<float>(i) / stacks;

      for (int j = 0; j <= slices; ++j) {
        float u = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
        Point3 pos = computeConePosition(u, v);
        Point3 normal = computeConeNormal(u, v);
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
      }
    }

    // compute the cone body indices
    for (int i = 0; i < stacks; ++i) {
      for (int j = 0; j < slices; ++j) {
        unsigned int v0 = (i * (slices + 1)) + j;
        unsigned int v1 = v0 + 1;
        unsigned int v2 = (i + 1) * (slices + 1) + j;
        unsigned int v3 = v2 + 1;
        // Note: The cone body is made of quads, so we need to create two triangles for each quad
        indices.push_back(v0);
        indices.push_back(v2);
        indices.push_back(v1);
        indices.push_back(v1);
        indices.push_back(v2);
        indices.push_back(v3);
      }
    }

    // compute the cone cap vertices
    for (int i = 0; i <= capStacks; ++i) {
      float v = static_cast<float>(i) / capStacks;
      for (int j = 0; j <= slices; ++j) {
        float u = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
        Point3 pos = computeCapPosition(u, v);
        Point3 normal = computeCapNormal(u, v);
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
      }
    }

    // compute the cone cap indices
    unsigned int capStart = (stacks + 1) * (slices + 1);
    for (int i = 0; i < capStacks; ++i) {
      for (int j = 0; j < slices; ++j) {
        unsigned int v0 = capStart + (i * (slices + 1)) + j;
        unsigned int v1 = v0 + 1;
        unsigned int v2 = capStart + ((i + 1) * (slices + 1)) + j;
        unsigned int v3 = v2 + 1;
        // Note: The cone cap is made of quads, so we need to create two triangles for each quad
        indices.push_back(v0);
        indices.push_back(v2);
        indices.push_back(v1);
        indices.push_back(v1);
        indices.push_back(v2);
        indices.push_back(v3);
      }
    }
  }
private:
  float x;
  float y;
  float capRadius;
  float radius;
  float height;

  int stacks;
  int capStacks;
};