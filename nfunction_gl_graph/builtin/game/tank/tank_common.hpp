#ifndef __TANK_COMMON_HPP_
#define __TANK_COMMON_HPP_

#include <algorithm>
#include <cmath>
#include <vector>
#include "../../../gl_type.h"

/**
 * Shared building blocks for the tank parts (hull, turret, gun).
 *
 * Coordinate system (matches the app camera, +Y is up):
 *   +X: forward
 *   +Y: up
 *   +Z: right side of the tank
 *
 * Meshes are triangle lists with interleaved position and normal (6 floats per vertex)
 * and a separate RGBA color array, ready for VboObject::initialize(..., normalOffset = 3).
 */

struct TankColor {
  float r;
  float g;
  float b;
  float a;

  TankColor() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
  TankColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

  TankColor shade(float k) const {
    return TankColor(std::min(r * k, 1.0f), std::min(g * k, 1.0f), std::min(b * k, 1.0f), a);
  }
};

struct TankPalette {
  TankColor hull;
  TankColor hullDark;
  TankColor hullLight;
  TankColor track;
  TankColor rubber;
  TankColor steel;
  TankColor darkMetal;
  TankColor glass;
  TankColor lens;
  TankColor tailLight;
  TankColor canvas;
  TankColor exhaust;

  static TankPalette fromHullColor(float r, float g, float b) {
    TankPalette p;
    p.hull = TankColor(r, g, b);
    p.hullDark = p.hull.shade(0.75f);
    p.hullLight = p.hull.shade(1.2f);
    p.track = TankColor(0.21f, 0.20f, 0.19f);
    p.rubber = TankColor(0.07f, 0.07f, 0.07f);
    p.steel = TankColor(0.50f, 0.50f, 0.48f);
    p.darkMetal = TankColor(0.24f, 0.24f, 0.23f);
    p.glass = TankColor(0.10f, 0.20f, 0.32f);
    p.lens = TankColor(0.98f, 0.90f, 0.60f);
    p.tailLight = TankColor(0.70f, 0.08f, 0.06f);
    p.canvas = TankColor(0.55f, 0.50f, 0.36f);
    p.exhaust = TankColor(0.36f, 0.24f, 0.16f);
    return p;
  }

  static TankPalette defaultPalette() {
    return fromHullColor(0.33f, 0.38f, 0.22f);
  }
};

/**
 * Rigid transform: p' = m * p + t
 */
struct TankTransform {
  float m[3][3];
  Point3 t;

  static TankTransform identity() {
    TankTransform x;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        x.m[i][j] = (i == j) ? 1.0f : 0.0f;
      }
    }
    x.t = Point3(0.0f, 0.0f, 0.0f);
    return x;
  }

  static TankTransform translation(const Point3 &offset) {
    TankTransform x = identity();
    x.t = offset;
    return x;
  }

  /** Rotation about +Y. Positive angle turns +X toward -Z. */
  static TankTransform rotationY(float angle) {
    TankTransform x = identity();
    float c = cos(angle);
    float s = sin(angle);
    x.m[0][0] = c;  x.m[0][2] = s;
    x.m[2][0] = -s; x.m[2][2] = c;
    return x;
  }

  /** Rotation about +Z. Positive angle turns +X toward +Y (raises the gun). */
  static TankTransform rotationZ(float angle) {
    TankTransform x = identity();
    float c = cos(angle);
    float s = sin(angle);
    x.m[0][0] = c; x.m[0][1] = -s;
    x.m[1][0] = s; x.m[1][1] = c;
    return x;
  }

  TankTransform operator*(const TankTransform &o) const {
    TankTransform x;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        x.m[i][j] = m[i][0] * o.m[0][j] + m[i][1] * o.m[1][j] + m[i][2] * o.m[2][j];
      }
    }
    x.t = applyVector(o.t) + t;
    return x;
  }

  Point3 applyVector(const Point3 &v) const {
    return Point3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                  m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                  m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
  }

  Point3 applyPoint(const Point3 &p) const {
    return applyVector(p) + t;
  }
};

inline float tankDot(const Point3 &a, const Point3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Point3 tankWithZ(const Point3 &p, float z) {
  return Point3(p.x, p.y, z);
}

/**
 * Appends primitives to shared vertex / index / color arrays.
 * Every primitive is placed through the current transform.
 */
class TankMeshBuilder {
public:
  TankTransform transform;

  TankMeshBuilder(std::vector<float> &vertices, std::vector<unsigned short> &indices, std::vector<float> &colors)
    : transform(TankTransform::identity()), vertices(vertices), indices(indices), colors(colors) {}

  unsigned int vertexCount() const {
    return static_cast<unsigned int>(vertices.size() / 6);
  }

  unsigned int addVertex(const Point3 &position, const Point3 &normal, const TankColor &color) {
    Point3 p = transform.applyPoint(position);
    Point3 n = transform.applyVector(normal);
    vertices.push_back(p.x);
    vertices.push_back(p.y);
    vertices.push_back(p.z);
    vertices.push_back(n.x);
    vertices.push_back(n.y);
    vertices.push_back(n.z);
    colors.push_back(color.r);
    colors.push_back(color.g);
    colors.push_back(color.b);
    colors.push_back(color.a);
    return vertexCount() - 1;
  }

  /**
   * The fragment shader flips the normal on back faces, so the winding must agree
   * with the vertex normals. The triangle is re-wound here when it does not.
   */
  void addTriangle(unsigned int a, unsigned int b, unsigned int c) {
    Point3 pa = position(a);
    Point3 geometric = (position(b) - pa).cross(position(c) - pa);
    Point3 reference = normal(a) + normal(b) + normal(c);
    if (tankDot(geometric, reference) < 0.0f) {
      std::swap(b, c);
    }
    indices.push_back(static_cast<unsigned short>(a));
    indices.push_back(static_cast<unsigned short>(b));
    indices.push_back(static_cast<unsigned short>(c));
  }

  void addQuad(const Point3 &p0, const Point3 &p1, const Point3 &p2, const Point3 &p3,
               const Point3 &n0, const Point3 &n1, const Point3 &n2, const Point3 &n3, const TankColor &color) {
    unsigned int i0 = addVertex(p0, n0, color);
    unsigned int i1 = addVertex(p1, n1, color);
    unsigned int i2 = addVertex(p2, n2, color);
    unsigned int i3 = addVertex(p3, n3, color);
    addTriangle(i0, i1, i2);
    addTriangle(i0, i2, i3);
  }

  void addFlatQuad(const Point3 &p0, const Point3 &p1, const Point3 &p2, const Point3 &p3,
                   const Point3 &normal, const TankColor &color) {
    addQuad(p0, p1, p2, p3, normal, normal, normal, normal, color);
  }

  /**
   * Convex solid with 8 corners: 0..3 bottom ring, 4..7 top ring (corner i+4 above corner i).
   * Faces are flat shaded. Each face normal points away from the solid's centroid.
   */
  void addHexahedron(const Point3 corners[8], const TankColor &color) {
    static const int faces[6][4] = {
      {0, 1, 2, 3}, {4, 5, 6, 7},
      {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}
    };
    Point3 center(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 8; ++i) {
      center = center + corners[i];
    }
    center = center * 0.125f;

    for (int f = 0; f < 6; ++f) {
      const Point3 &p0 = corners[faces[f][0]];
      const Point3 &p1 = corners[faces[f][1]];
      const Point3 &p2 = corners[faces[f][2]];
      const Point3 &p3 = corners[faces[f][3]];
      Point3 n = (p2 - p0).cross(p3 - p1);
      if (tankDot(n, n) < 1e-12f) {
        continue;
      }
      Point3 faceCenter = (p0 + p1 + p2 + p3) * 0.25f;
      if (tankDot(n, faceCenter - center) < 0.0f) {
        n = n * -1.0f;
      }
      addFlatQuad(p0, p1, p2, p3, n.normalize(), color);
    }
  }

  /** Box with arbitrary orthonormal axes and half extents along them. */
  void addOrientedBox(const Point3 &c, const Point3 &ax, const Point3 &ay, const Point3 &az,
                      float hx, float hy, float hz, const TankColor &color) {
    Point3 x = ax * hx;
    Point3 y = ay * hy;
    Point3 z = az * hz;
    Point3 corners[8] = {
      c - x - y - z, c + x - y - z, c + x - y + z, c - x - y + z,
      c - x + y - z, c + x + y - z, c + x + y + z, c - x + y + z
    };
    addHexahedron(corners, color);
  }

  void addBox(const Point3 &center, float hx, float hy, float hz, const TankColor &color) {
    addOrientedBox(center, Point3(1.0f, 0.0f, 0.0f), Point3(0.0f, 1.0f, 0.0f), Point3(0.0f, 0.0f, 1.0f),
                   hx, hy, hz, color);
  }

  /**
   * Truncated cone around an arbitrary axis.
   *   P(u, h) = base + axis * h + (U cos u + V sin u) * (r0 + (r1 - r0) * h / length)
   *   u in [0, 2*PI], h in [0, length], U and V span the plane perpendicular to the axis
   * Normal: normalize(radial * length + axis * (r0 - r1))
   */
  void addFrustum(const Point3 &base, const Point3 &axis, float length, float r0, float r1, int slices,
                  const TankColor &color, bool capStart, bool capEnd) {
    Point3 a = axis.normalize();
    Point3 helper = (fabs(a.y) < 0.9f) ? Point3(0.0f, 1.0f, 0.0f) : Point3(1.0f, 0.0f, 0.0f);
    Point3 u = helper.cross(a).normalize();
    Point3 v = a.cross(u);
    Point3 top = base + a * length;

    unsigned int start = vertexCount();
    for (int j = 0; j <= slices; ++j) {
      float angle = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
      Point3 radial = u * cos(angle) + v * sin(angle);
      Point3 n = (radial * length + a * (r0 - r1)).normalize();
      addVertex(base + radial * r0, n, color);
      addVertex(top + radial * r1, n, color);
    }
    for (int j = 0; j < slices; ++j) {
      unsigned int b0 = start + 2 * j;
      unsigned int t0 = b0 + 1;
      unsigned int b1 = b0 + 2;
      unsigned int t1 = b0 + 3;
      addTriangle(b0, b1, t0);
      addTriangle(b1, t1, t0);
    }

    if (capStart && r0 > 0.0f) {
      addDisk(base, a * -1.0f, u, v, r0, slices, color);
    }
    if (capEnd && r1 > 0.0f) {
      addDisk(top, a, u, v, r1, slices, color);
    }
  }

  void addRod(const Point3 &from, const Point3 &to, float radius, int slices, const TankColor &color) {
    Point3 d = to - from;
    float length = sqrt(tankDot(d, d));
    if (length <= 0.0f) {
      return;
    }
    addFrustum(from, d, length, radius, radius, slices, color, true, true);
  }

private:
  std::vector<float> &vertices;
  std::vector<unsigned short> &indices;
  std::vector<float> &colors;

  Point3 position(unsigned int i) const {
    return Point3(vertices[i * 6], vertices[i * 6 + 1], vertices[i * 6 + 2]);
  }

  Point3 normal(unsigned int i) const {
    return Point3(vertices[i * 6 + 3], vertices[i * 6 + 4], vertices[i * 6 + 5]);
  }

  void addDisk(const Point3 &center, const Point3 &n, const Point3 &u, const Point3 &v, float radius,
               int slices, const TankColor &color) {
    unsigned int c = addVertex(center, n, color);
    for (int j = 0; j <= slices; ++j) {
      float angle = 2.0f * M_PI * static_cast<float>(j) / static_cast<float>(slices);
      addVertex(center + (u * cos(angle) + v * sin(angle)) * radius, n, color);
    }
    for (int j = 0; j < slices; ++j) {
      addTriangle(c, c + 1 + j, c + 2 + j);
    }
  }
};

/**
 * Builds one part alone, in its own local frame.
 * Part must provide: void build(TankMeshBuilder &b) const
 * Return: number of vertices generated
 */
template <typename Part>
int generateTankPartMesh(const Part &part, std::vector<float> &vertices, std::vector<unsigned short> &indices,
                         std::vector<float> &colors) {
  TankMeshBuilder b(vertices, indices, colors);
  part.build(b);
  return static_cast<int>(b.vertexCount());
}

#endif // __TANK_COMMON_HPP_
