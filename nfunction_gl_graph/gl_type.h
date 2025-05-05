#ifndef _GL_TYPE_H_
#define _GL_TYPE_H_

#include <cmath>

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

#endif