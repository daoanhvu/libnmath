#ifndef __TANK_HPP_
#define __TANK_HPP_

#include <algorithm>
#include <cmath>
#include <vector>
#include "../../gl_type.h"

/**
 * Tank model built from parametric primitives (boxes, hexahedra, frustums, track loop).
 *
 * Coordinate system (matches the app camera, +Y is up):
 *   +X: forward (direction of the cannon at turretYaw = 0)
 *   +Y: up, the tracks sit on the ground plane y = 0
 *   +Z: right side of the tank
 *
 * Output is one triangle list with interleaved position and normal (6 floats per vertex)
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

class Tank {
public:
  /**
   * Param turretYaw: turret rotation around +Y in radians (0 = cannon points along +X)
   * Param gunPitch: cannon elevation in radians, clamped to [-10, +20] degrees
   * Param palette: colors, TankPalette::fromHullColor() derives the shades from one base color
   */
  Tank(float turretYaw = 0.0f, float gunPitch = 0.0f,
       const TankPalette &palette = TankPalette::fromHullColor(0.33f, 0.38f, 0.22f))
    : turretYaw(turretYaw),
      gunPitch(std::max(-0.1745f, std::min(gunPitch, 0.349f))),
      palette(palette) {}

  /**
   * Generate the whole tank as one triangle list
   * Param vertices: output, 6 floats per vertex (x, y, z, nx, ny, nz)
   * Param indices: output, 3 indices per triangle (GL_TRIANGLES)
   * Param colors: output, 4 floats per vertex (r, g, b, a)
   * Return: number of vertices generated
   */
  int generateMesh(std::vector<float> &vertices, std::vector<unsigned short> &indices,
                   std::vector<float> &colors) const {
    TankMeshBuilder b(vertices, indices, colors);
    buildRunningGear(b, -1.0f);
    buildRunningGear(b, 1.0f);
    buildHull(b);
    buildTurret(b);
    return static_cast<int>(b.vertexCount());
  }

private:
  static constexpr float grouserHeight = 0.04f;

  float turretYaw;
  float gunPitch;
  TankPalette palette;

  struct TrackWheel {
    float x;
    float y;
    float r;
  };

  struct TrackSample {
    float x;
    float y;
    float nx;
    float ny;
  };

  /**
   * Inner surface of the track: a closed belt wrapped around the given wheels.
   * Wheels must be listed counter-clockwise as seen from +Z.
   * Between wheel A and wheel B the belt follows their outer tangent line. Its normal n satisfies
   *   n . (B - A) = rA - rB,  so  n = right(d) * sqrt(1 - s^2) + d * s,  s = (rA - rB) / |B - A|
   * Around each wheel the belt is the arc between the incoming and outgoing tangent normals.
   */
  static std::vector<TrackSample> buildTrackPath(const std::vector<TrackWheel> &wheels, float arcStep, float lineStep) {
    size_t count = wheels.size();
    std::vector<float> nx(count);
    std::vector<float> ny(count);
    for (size_t i = 0; i < count; ++i) {
      const TrackWheel &a = wheels[i];
      const TrackWheel &b = wheels[(i + 1) % count];
      float dx = b.x - a.x;
      float dy = b.y - a.y;
      float d = sqrt(dx * dx + dy * dy);
      dx /= d;
      dy /= d;
      float s = (a.r - b.r) / d;
      float c = sqrt(std::max(0.0f, 1.0f - s * s));
      nx[i] = dy * c + dx * s;
      ny[i] = -dx * c + dy * s;
    }

    std::vector<TrackSample> samples;
    for (size_t i = 0; i < count; ++i) {
      const TrackWheel &w = wheels[i];
      size_t prev = (i + count - 1) % count;
      float a0 = atan2(ny[prev], nx[prev]);
      float a1 = atan2(ny[i], nx[i]);
      if (a1 < a0 - 1e-4f) {
        a1 += 2.0f * M_PI;
      } else if (a1 < a0) {
        a1 = a0;
      }
      int arcSteps = std::max(1, static_cast<int>(ceil((a1 - a0) / arcStep)));
      for (int k = 0; k < arcSteps; ++k) {
        float angle = a0 + (a1 - a0) * static_cast<float>(k) / static_cast<float>(arcSteps);
        float cx = cos(angle);
        float cy = sin(angle);
        samples.push_back({w.x + cx * w.r, w.y + cy * w.r, cx, cy});
      }

      const TrackWheel &next = wheels[(i + 1) % count];
      float px = w.x + nx[i] * w.r;
      float py = w.y + ny[i] * w.r;
      float qx = next.x + nx[i] * next.r;
      float qy = next.y + ny[i] * next.r;
      float length = sqrt((qx - px) * (qx - px) + (qy - py) * (qy - py));
      int lineSteps = std::max(1, static_cast<int>(ceil(length / lineStep)));
      for (int k = 0; k < lineSteps; ++k) {
        float t = static_cast<float>(k) / static_cast<float>(lineSteps);
        samples.push_back({px + (qx - px) * t, py + (qy - py) * t, nx[i], ny[i]});
      }
    }
    return samples;
  }

  void addTrackBand(TankMeshBuilder &b, const std::vector<TrackSample> &path, float zc, float width,
                    float thickness) const {
    float z0 = zc - width * 0.5f;
    float z1 = zc + width * 0.5f;
    Point3 sideNormal(0.0f, 0.0f, 1.0f);
    size_t n = path.size();
    for (size_t i = 0; i < n; ++i) {
      const TrackSample &p = path[i];
      const TrackSample &q = path[(i + 1) % n];
      Point3 pn(p.nx, p.ny, 0.0f);
      Point3 qn(q.nx, q.ny, 0.0f);
      Point3 pIn(p.x, p.y, 0.0f);
      Point3 qIn(q.x, q.y, 0.0f);
      Point3 pOut = pIn + pn * thickness;
      Point3 qOut = qIn + qn * thickness;

      b.addQuad(tankWithZ(pOut, z0), tankWithZ(qOut, z0), tankWithZ(qOut, z1), tankWithZ(pOut, z1),
                pn, qn, qn, pn, palette.track);
      b.addQuad(tankWithZ(pIn, z0), tankWithZ(qIn, z0), tankWithZ(qIn, z1), tankWithZ(pIn, z1),
                pn * -1.0f, qn * -1.0f, qn * -1.0f, pn * -1.0f, palette.track.shade(0.8f));
      b.addFlatQuad(tankWithZ(pIn, z1), tankWithZ(qIn, z1), tankWithZ(qOut, z1), tankWithZ(pOut, z1),
                    sideNormal, palette.track);
      b.addFlatQuad(tankWithZ(pIn, z0), tankWithZ(qIn, z0), tankWithZ(qOut, z0), tankWithZ(pOut, z0),
                    sideNormal * -1.0f, palette.track);
    }
  }

  /**
   * Track shoes spaced evenly by arc length along the belt: a grouser on the outside,
   * steel end connectors on both edges and a guide horn on the inside.
   */
  void addTrackShoes(TankMeshBuilder &b, const std::vector<TrackSample> &path, float zc, float width,
                     float thickness) const {
    size_t n = path.size();
    std::vector<float> cumulative(n + 1, 0.0f);
    for (size_t i = 0; i < n; ++i) {
      const TrackSample &p = path[i];
      const TrackSample &q = path[(i + 1) % n];
      cumulative[i + 1] = cumulative[i] + sqrt((q.x - p.x) * (q.x - p.x) + (q.y - p.y) * (q.y - p.y));
    }
    float total = cumulative[n];
    int shoes = std::max(1, static_cast<int>(round(total / 0.2f)));
    Point3 axisZ(0.0f, 0.0f, 1.0f);
    size_t seg = 0;
    for (int k = 0; k < shoes; ++k) {
      float target = total * static_cast<float>(k) / static_cast<float>(shoes);
      while (seg + 1 < n && cumulative[seg + 1] < target) {
        ++seg;
      }
      const TrackSample &p = path[seg];
      const TrackSample &q = path[(seg + 1) % n];
      float segLength = cumulative[seg + 1] - cumulative[seg];
      float t = (segLength > 0.0f) ? (target - cumulative[seg]) / segLength : 0.0f;
      Point3 pos(p.x + (q.x - p.x) * t, p.y + (q.y - p.y) * t, 0.0f);
      Point3 normal = Point3(p.nx + (q.nx - p.nx) * t, p.ny + (q.ny - p.ny) * t, 0.0f).normalize();
      Point3 tangent(-normal.y, normal.x, 0.0f);

      b.addOrientedBox(tankWithZ(pos + normal * (thickness + grouserHeight * 0.5f), zc), tangent, normal, axisZ,
                       0.055f, grouserHeight * 0.5f, width * 0.47f, palette.track.shade(0.85f));
      for (float side = -1.0f; side <= 1.0f; side += 2.0f) {
        b.addOrientedBox(tankWithZ(pos + normal * (thickness * 0.5f), zc + side * (width * 0.5f + 0.015f)),
                         tangent, normal, axisZ, 0.045f, thickness * 0.55f, 0.018f, palette.steel.shade(0.7f));
      }
      b.addOrientedBox(tankWithZ(pos - normal * 0.045f, zc), tangent, normal, axisZ,
                       0.035f, 0.045f, 0.03f, palette.track);
    }
  }

  /**
   * Double road wheel: two rubber-tired discs with a gap for the guide horns,
   * painted outer disc, hub cap and a bolt circle on the outside.
   * Param s: +1 for the right side (outside is +Z), -1 for the left side
   */
  void addRoadWheel(TankMeshBuilder &b, float cx, float cy, float zc, float s, float radius) const {
    Point3 out(0.0f, 0.0f, s);
    Point3 in(0.0f, 0.0f, -s);
    Point3 c(cx, cy, zc);
    const float gap = 0.04f;
    const float tire = 0.22f;
    const float face = gap + tire;

    b.addFrustum(c + out * gap, out, tire, radius, radius, 24, palette.rubber, true, true);
    b.addFrustum(c + in * gap, in, tire, radius, radius, 24, palette.rubber, true, true);
    b.addFrustum(c + in * gap, out, gap * 2.0f, radius * 0.45f, radius * 0.45f, 12, palette.darkMetal, false, false);
    b.addFrustum(c + in * face, in, 0.015f, radius * 0.78f, radius * 0.76f, 20, palette.hullDark, false, true);

    b.addFrustum(c + out * face, out, 0.02f, radius * 0.78f, radius * 0.74f, 24, palette.hull, false, true);
    b.addFrustum(c + out * (face + 0.02f), out, 0.06f, radius * 0.3f, radius * 0.22f, 12, palette.steel, false, true);
    for (int k = 0; k < 6; ++k) {
      float angle = 2.0f * M_PI * static_cast<float>(k) / 6.0f;
      Point3 p = c + Point3(cos(angle) * radius * 0.5f, sin(angle) * radius * 0.5f, 0.0f) + out * (face + 0.02f);
      b.addFrustum(p, out, 0.025f, 0.018f, 0.018f, 6, palette.steel.shade(0.8f), false, true);
    }
  }

  /**
   * Rear drive sprocket: axle hub and two toothed rings that straddle the guide horns.
   */
  void addSprocket(TankMeshBuilder &b, float cx, float cy, float zc, float s, float radius) const {
    Point3 out(0.0f, 0.0f, s);
    Point3 axisZ(0.0f, 0.0f, 1.0f);
    Point3 c(cx, cy, zc);
    const int teeth = 11;

    b.addFrustum(c - out * 0.22f, out, 0.44f, 0.14f, 0.14f, 16, palette.darkMetal, true, true);
    for (int ring = 0; ring < 2; ++ring) {
      Point3 ringCenter = c + out * (ring == 0 ? -0.15f : 0.15f);
      b.addFrustum(ringCenter - out * 0.04f, out, 0.08f, radius - 0.06f, radius - 0.06f, 24,
                   palette.hullDark, true, true);
      for (int k = 0; k < teeth; ++k) {
        float angle = 2.0f * M_PI * static_cast<float>(k) / static_cast<float>(teeth);
        Point3 radial(cos(angle), sin(angle), 0.0f);
        Point3 tangent(-sin(angle), cos(angle), 0.0f);
        b.addOrientedBox(ringCenter + radial * (radius - 0.03f), radial, tangent, axisZ,
                         0.05f, 0.035f, 0.04f, palette.darkMetal);
      }
    }
    b.addFrustum(c + out * 0.19f, out, 0.07f, 0.13f, 0.09f, 16, palette.steel, false, true);
    for (int k = 0; k < 8; ++k) {
      float angle = 2.0f * M_PI * static_cast<float>(k) / 8.0f;
      Point3 p = c + Point3(cos(angle) * 0.2f, sin(angle) * 0.2f, 0.0f) + out * 0.19f;
      b.addFrustum(p, out, 0.025f, 0.018f, 0.018f, 6, palette.steel.shade(0.8f), false, true);
    }
  }

  /**
   * Tracks, road wheels, idler, sprocket, return rollers and suspension arms for one side.
   * Param s: +1 for the right side (+Z), -1 for the left side
   */
  void buildRunningGear(TankMeshBuilder &b, float s) const {
    const float zc = s * 1.5f;
    const float trackWidth = 0.62f;
    const float trackThickness = 0.08f;
    const float wheelRadius = 0.36f;
    const float wheelY = grouserHeight + trackThickness + wheelRadius;
    const float wheelX[6] = {-2.1f, -1.26f, -0.42f, 0.42f, 1.26f, 2.1f};
    const TrackWheel sprocket = {-2.85f, 0.78f, 0.34f};
    const TrackWheel idler = {2.9f, 0.74f, 0.30f};

    for (int i = 0; i < 6; ++i) {
      addRoadWheel(b, wheelX[i], wheelY, zc, s, wheelRadius);
      b.addRod(Point3(wheelX[i] + 0.32f, 0.62f, s * 1.12f), Point3(wheelX[i], wheelY, s * 1.25f),
               0.045f, 8, palette.darkMetal);
    }
    addRoadWheel(b, idler.x, idler.y, zc, s, idler.r);
    addSprocket(b, sprocket.x, sprocket.y, zc, s, sprocket.r);

    const float rollerX[3] = {-1.4f, 0.0f, 1.4f};
    for (int i = 0; i < 3; ++i) {
      float x = rollerX[i];
      float topRunY = idler.y + idler.r + (idler.x - x) / (idler.x - sprocket.x) *
                      ((sprocket.y + sprocket.r) - (idler.y + idler.r));
      float y = topRunY - 0.105f;
      for (int half = 0; half < 2; ++half) {
        float offset = (half == 0) ? -0.13f : 0.13f;
        b.addFrustum(Point3(x, y, zc + offset - 0.06f), Point3(0.0f, 0.0f, 1.0f), 0.12f, 0.1f, 0.1f, 16,
                     palette.rubber, true, true);
      }
      b.addRod(Point3(x, y, s * 1.12f), Point3(x, y, zc + s * 0.19f), 0.04f, 8, palette.darkMetal);
    }

    std::vector<TrackWheel> wheels = {
      {wheelX[0], wheelY, wheelRadius},
      {wheelX[5], wheelY, wheelRadius},
      idler,
      sprocket
    };
    std::vector<TrackSample> path = buildTrackPath(wheels, 0.17f, 0.25f);
    addTrackBand(b, path, zc, trackWidth, trackThickness);
    addTrackShoes(b, path, zc, trackWidth, trackThickness);
  }

  void buildHull(TankMeshBuilder &b) const {
    const Point3 X(1.0f, 0.0f, 0.0f);
    const Point3 Y(0.0f, 1.0f, 0.0f);
    const Point3 Z(0.0f, 0.0f, 1.0f);

    // Lower hull between the tracks, with sloped lower front and rear plates
    Point3 lower[8] = {
      {-2.75f, 0.40f, -1.15f}, {2.55f, 0.40f, -1.15f}, {2.55f, 0.40f, 1.15f}, {-2.75f, 0.40f, 1.15f},
      {-3.00f, 1.24f, -1.15f}, {3.05f, 1.24f, -1.15f}, {3.05f, 1.24f, 1.15f}, {-3.00f, 1.24f, 1.15f}
    };
    b.addHexahedron(lower, palette.hullDark);

    // Upper hull over the tracks: long shallow glacis in front, sloped side walls
    Point3 upper[8] = {
      {-3.05f, 1.24f, -1.86f}, {3.20f, 1.24f, -1.86f}, {3.20f, 1.24f, 1.86f}, {-3.05f, 1.24f, 1.86f},
      {-2.95f, 1.78f, -1.60f}, {1.80f, 1.78f, -1.60f}, {1.80f, 1.78f, 1.60f}, {-2.95f, 1.78f, 1.60f}
    };
    b.addHexahedron(upper, palette.hull);

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      float z0 = s * 1.17f;
      float z1 = s * 1.83f;

      // Mud flaps over the front and rear of the track
      Point3 frontFlap[8] = {
        {3.20f, 1.21f, z0}, {3.42f, 1.09f, z0}, {3.42f, 1.09f, z1}, {3.20f, 1.21f, z1},
        {3.20f, 1.24f, z0}, {3.42f, 1.12f, z0}, {3.42f, 1.12f, z1}, {3.20f, 1.24f, z1}
      };
      b.addHexahedron(frontFlap, palette.hullDark);
      Point3 rearFlap[8] = {
        {-3.05f, 1.21f, z0}, {-3.22f, 1.12f, z0}, {-3.22f, 1.12f, z1}, {-3.05f, 1.21f, z1},
        {-3.05f, 1.24f, z0}, {-3.22f, 1.15f, z0}, {-3.22f, 1.15f, z1}, {-3.05f, 1.24f, z1}
      };
      b.addHexahedron(rearFlap, palette.hullDark);

      // Armored side skirts: six panels with small gaps, the front one is raked
      const float skirtStart = -2.75f;
      const float panelLength = 0.93f;
      const float panelGap = 0.024f;
      float zIn = s * 1.88f;
      float zOut = s * 1.92f;
      for (int i = 0; i < 6; ++i) {
        float x0 = skirtStart + i * (panelLength + panelGap);
        float x1 = x0 + panelLength;
        float bottomX1 = (i == 5) ? x1 - 0.18f : x1;
        Point3 panel[8] = {
          {x0, 0.84f, zIn}, {bottomX1, 0.84f, zIn}, {bottomX1, 0.84f, zOut}, {x0, 0.84f, zOut},
          {x0, 1.30f, zIn}, {x1, 1.30f, zIn}, {x1, 1.30f, zOut}, {x0, 1.30f, zOut}
        };
        b.addHexahedron(panel, (i % 2 == 0) ? palette.hull : palette.hull.shade(0.93f));
        b.addBox(Point3((x0 + bottomX1) * 0.5f, 0.82f, s * 1.90f), (bottomX1 - x0) * 0.5f, 0.02f, 0.015f,
                 palette.rubber);
      }

      // Spare track links bolted on the glacis
      const Point3 glacisOrigin(3.20f, 1.24f, 0.0f);
      const Point3 slope = Point3(-1.40f, 0.54f, 0.0f).normalize();
      const Point3 glacisNormal = Point3(0.54f, 1.40f, 0.0f).normalize();
      for (int k = 0; k < 4; ++k) {
        Point3 c = tankWithZ(glacisOrigin + slope * (0.3f + 0.22f * k) + glacisNormal * 0.035f, s * 0.8f);
        b.addOrientedBox(c, slope, glacisNormal, Z, 0.09f, 0.03f, 0.27f, palette.track);
        b.addOrientedBox(c + glacisNormal * 0.05f, slope, glacisNormal, Z, 0.035f, 0.03f, 0.03f, palette.track);
      }

      // Headlights on stands at the front corners of the glacis
      Point3 light(2.92f, 1.43f, s * 1.42f);
      b.addBox(Point3(2.98f, 1.37f, s * 1.42f), 0.05f, 0.05f, 0.04f, palette.hullDark);
      b.addFrustum(light, X, 0.14f, 0.08f, 0.09f, 16, palette.hullDark, true, true);
      b.addFrustum(light + X * 0.14f, X, 0.01f, 0.07f, 0.07f, 16, palette.lens, false, true);

      // Rear deck stowage bins with lids
      b.addBox(Point3(-2.2f, 1.9f, s * 1.42f), 0.6f, 0.12f, 0.14f, palette.hullDark);
      b.addBox(Point3(-2.2f, 2.035f, s * 1.42f), 0.62f, 0.015f, 0.155f, palette.hull);

      // Engine deck grilles
      b.addBox(Point3(-2.175f, 1.79f, s * 0.7f), 0.625f, 0.012f, 0.55f, palette.darkMetal);
      for (int k = 0; k < 8; ++k) {
        float x = -2.72f + k * 0.155f;
        b.addBox(Point3(x, 1.81f, s * 0.7f), 0.04f, 0.015f, 0.52f, palette.hullDark);
      }

      // Fuel drums on the rear plate, with ribs and brackets
      Point3 drumBase(-3.22f, 1.5f, s * 0.55f - 0.375f);
      b.addFrustum(drumBase, Z, 0.75f, 0.2f, 0.2f, 20, palette.hullDark, true, true);
      for (int k = 0; k < 2; ++k) {
        b.addFrustum(drumBase + Z * (0.2f + 0.32f * k), Z, 0.03f, 0.212f, 0.212f, 20, palette.darkMetal, true, true);
      }
      b.addBox(Point3(-3.05f, 1.5f, s * 0.55f - 0.3f), 0.06f, 0.03f, 0.03f, palette.darkMetal);
      b.addBox(Point3(-3.05f, 1.5f, s * 0.55f + 0.3f), 0.06f, 0.03f, 0.03f, palette.darkMetal);

      // Exhaust outlets on the lower rear plate
      Point3 exhaust(-2.92f, 1.0f, s * 0.45f);
      b.addFrustum(exhaust, X * -1.0f, 0.18f, 0.075f, 0.075f, 14, palette.exhaust, false, true);
      b.addFrustum(exhaust - X * 0.181f, X * -1.0f, 0.003f, 0.055f, 0.055f, 14, palette.rubber, false, true);

      // Tail lights and tow hooks
      b.addBox(Point3(-3.0f, 1.62f, s * 1.5f), 0.03f, 0.05f, 0.08f, palette.tailLight);
      b.addBox(Point3(2.85f, 0.8f, s * 0.8f), 0.08f, 0.05f, 0.05f, palette.darkMetal);
      b.addBox(Point3(-2.93f, 0.75f, s * 0.8f), 0.08f, 0.05f, 0.05f, palette.darkMetal);
    }

    // Driver hatch and periscopes in front of the turret
    b.addFrustum(Point3(1.35f, 1.78f, 0.0f), Y, 0.05f, 0.25f, 0.24f, 20, palette.hullLight, false, true);
    b.addBox(Point3(1.35f, 1.85f, 0.0f), 0.03f, 0.02f, 0.1f, palette.darkMetal);
    for (int k = -1; k <= 1; ++k) {
      b.addBox(Point3(1.64f, 1.82f, k * 0.18f), 0.04f, 0.04f, 0.07f, palette.glass);
    }
  }

  void buildTurret(TankMeshBuilder &b) const {
    const Point3 X(1.0f, 0.0f, 0.0f);
    const Point3 Y(0.0f, 1.0f, 0.0f);
    const Point3 Z(0.0f, 0.0f, 1.0f);

    TankTransform turretTransform = TankTransform::translation(Point3(-0.25f, 1.78f, 0.0f)) *
                                    TankTransform::rotationY(turretYaw);
    b.transform = turretTransform;

    // Turret ring
    b.addFrustum(Point3(0.0f, 0.0f, 0.0f), Y, 0.1f, 1.05f, 1.05f, 32, palette.darkMetal, false, true);

    // Main body with sloped sides
    Point3 body[8] = {
      {-1.35f, 0.10f, -1.30f}, {1.05f, 0.10f, -1.30f}, {1.05f, 0.10f, 1.30f}, {-1.35f, 0.10f, 1.30f},
      {-1.25f, 0.72f, -1.05f}, {0.75f, 0.72f, -1.05f}, {0.75f, 0.72f, 1.05f}, {-1.25f, 0.72f, 1.05f}
    };
    b.addHexahedron(body, palette.hull);

    // Arrow-shaped front armor wedge
    Point3 wedge[8] = {
      {1.05f, 0.10f, -1.30f}, {1.75f, 0.10f, -0.42f}, {1.75f, 0.10f, 0.42f}, {1.05f, 0.10f, 1.30f},
      {0.75f, 0.72f, -1.05f}, {1.45f, 0.72f, -0.38f}, {1.45f, 0.72f, 0.38f}, {0.75f, 0.72f, 1.05f}
    };
    b.addHexahedron(wedge, palette.hullLight);

    // Rear bustle
    Point3 bustle[8] = {
      {-1.95f, 0.18f, -0.95f}, {-1.30f, 0.18f, -0.95f}, {-1.30f, 0.18f, 0.95f}, {-1.95f, 0.18f, 0.95f},
      {-2.15f, 0.70f, -0.90f}, {-1.25f, 0.70f, -0.90f}, {-1.25f, 0.70f, 0.90f}, {-2.15f, 0.70f, 0.90f}
    };
    b.addHexahedron(bustle, palette.hull);

    // Bustle rack with a rolled tarp and a crate
    const float rackX = -2.5f;
    const float rackR = 0.022f;
    const float levels[2] = {0.28f, 0.72f};
    for (int i = 0; i < 2; ++i) {
      float y = levels[i];
      b.addRod(Point3(-1.35f, y, -1.0f), Point3(rackX, y, -1.0f), rackR, 6, palette.darkMetal);
      b.addRod(Point3(-1.35f, y, 1.0f), Point3(rackX, y, 1.0f), rackR, 6, palette.darkMetal);
      b.addRod(Point3(rackX, y, -1.0f), Point3(rackX, y, 1.0f), rackR, 6, palette.darkMetal);
    }
    const float postZ[4] = {-1.0f, -0.33f, 0.33f, 1.0f};
    for (int i = 0; i < 4; ++i) {
      b.addRod(Point3(rackX, levels[0], postZ[i]), Point3(rackX, levels[1], postZ[i]), rackR, 6, palette.darkMetal);
    }
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      b.addRod(Point3(-1.9f, levels[0], s), Point3(-1.9f, levels[1], s), rackR, 6, palette.darkMetal);
    }
    b.addRod(Point3(-2.25f, levels[0], -1.0f), Point3(-2.25f, levels[0], 1.0f), rackR, 6, palette.darkMetal);
    b.addRod(Point3(-2.4f, levels[0], -1.0f), Point3(-2.4f, levels[0], 1.0f), rackR, 6, palette.darkMetal);
    Point3 tarpBase(-2.33f, 0.44f, -0.85f);
    b.addFrustum(tarpBase, Z, 1.7f, 0.15f, 0.15f, 16, palette.canvas, true, true);
    b.addFrustum(tarpBase + Z * 0.35f, Z, 0.04f, 0.157f, 0.157f, 16, palette.canvas.shade(0.6f), true, true);
    b.addFrustum(tarpBase + Z * 1.31f, Z, 0.04f, 0.157f, 0.157f, 16, palette.canvas.shade(0.6f), true, true);
    b.addBox(Point3(-2.33f, 0.67f, 0.4f), 0.13f, 0.08f, 0.25f, palette.hullDark);

    // Commander cupola with vision blocks and hatch
    Point3 cupola(-0.55f, 0.72f, -0.55f);
    b.addFrustum(cupola, Y, 0.2f, 0.36f, 0.33f, 24, palette.hull, false, true);
    for (int k = 0; k < 7; ++k) {
      float angle = 2.0f * M_PI * static_cast<float>(k) / 7.0f;
      Point3 dir(cos(angle), 0.0f, sin(angle));
      Point3 tangent(-sin(angle), 0.0f, cos(angle));
      b.addOrientedBox(cupola + Y * 0.11f + dir * 0.345f, dir, Y, tangent, 0.03f, 0.045f, 0.065f, palette.glass);
    }
    b.addFrustum(cupola + Y * 0.2f, Y, 0.05f, 0.29f, 0.27f, 24, palette.hullLight, false, true);
    b.addBox(cupola + Y * 0.27f, 0.12f, 0.02f, 0.025f, palette.darkMetal);

    // Loader hatch and pintle-mounted machine gun
    Point3 loader(-0.55f, 0.72f, 0.55f);
    b.addFrustum(loader, Y, 0.06f, 0.32f, 0.31f, 24, palette.hullDark, false, true);
    b.addFrustum(loader + Y * 0.06f, Y, 0.04f, 0.27f, 0.26f, 24, palette.hullLight, false, true);
    b.addRod(Point3(-0.2f, 0.72f, 0.92f), Point3(-0.2f, 0.98f, 0.92f), 0.03f, 8, palette.darkMetal);
    b.addBox(Point3(-0.12f, 1.02f, 0.92f), 0.18f, 0.05f, 0.045f, palette.darkMetal);
    b.addRod(Point3(0.06f, 1.03f, 0.92f), Point3(0.75f, 1.03f, 0.92f), 0.022f, 8, palette.darkMetal);
    b.addFrustum(Point3(0.6f, 1.03f, 0.92f), X, 0.15f, 0.03f, 0.03f, 8, palette.darkMetal, true, true);
    b.addBox(Point3(-0.1f, 0.95f, 1.0f), 0.07f, 0.05f, 0.04f, palette.hullDark);

    // Gunner primary sight
    b.addBox(Point3(0.35f, 0.86f, 0.68f), 0.2f, 0.14f, 0.17f, palette.hull);
    b.addBox(Point3(0.555f, 0.88f, 0.68f), 0.01f, 0.09f, 0.12f, palette.glass);

    // Commander panoramic sight
    b.addFrustum(Point3(0.15f, 0.72f, -0.75f), Y, 0.2f, 0.1f, 0.1f, 16, palette.hullDark, false, false);
    b.addBox(Point3(0.15f, 1.0f, -0.75f), 0.13f, 0.1f, 0.13f, palette.hull);
    b.addBox(Point3(0.285f, 1.0f, -0.75f), 0.008f, 0.06f, 0.09f, palette.glass);

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      // Smoke grenade launchers: 2 x 2 tubes angled forward, up and outward
      Point3 launcher(0.35f, 0.5f, s * 1.18f);
      b.addBox(launcher, 0.14f, 0.09f, 0.06f, palette.hullDark);
      Point3 tubeDir = Point3(0.5f, 0.45f, s * 0.74f).normalize();
      for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
          Point3 base = launcher + X * (-0.06f + 0.12f * i) + Y * (-0.04f + 0.08f * j);
          b.addFrustum(base, tubeDir, 0.26f, 0.042f, 0.042f, 10, palette.darkMetal, true, false);
          b.addFrustum(base + tubeDir * 0.26f, tubeDir, 0.004f, 0.032f, 0.032f, 10, palette.rubber, false, true);
        }
      }

      // Side stowage boxes
      b.addBox(Point3(-0.6f, 0.38f, s * 1.3f), 0.5f, 0.18f, 0.11f, palette.hullDark);

      // Whip antennas at the rear corners
      Point3 antenna(-1.55f, 0.7f, s * 0.72f);
      b.addFrustum(antenna, Y, 0.08f, 0.06f, 0.05f, 10, palette.darkMetal, false, true);
      b.addFrustum(antenna + Y * 0.08f, Point3(-0.12f, 1.0f, 0.0f), 1.5f, 0.012f, 0.006f, 6, palette.darkMetal,
                   false, true);
    }

    buildCannon(b, turretTransform);
    b.transform = TankTransform::identity();
  }

  /**
   * Cannon in its own frame: origin at the trunnion, barrel along +X.
   */
  void buildCannon(TankMeshBuilder &b, const TankTransform &turretTransform) const {
    const Point3 X(1.0f, 0.0f, 0.0f);
    b.transform = turretTransform * TankTransform::translation(Point3(1.72f, 0.4f, 0.0f)) *
                  TankTransform::rotationZ(gunPitch);

    // Mantlet and coaxial machine gun port
    b.addBox(Point3(0.1f, 0.0f, 0.0f), 0.17f, 0.25f, 0.36f, palette.hull);
    b.addFrustum(Point3(0.26f, 0.08f, -0.22f), X, 0.06f, 0.035f, 0.035f, 10, palette.darkMetal, false, true);
    b.addFrustum(Point3(0.321f, 0.08f, -0.22f), X, 0.002f, 0.02f, 0.02f, 10, palette.rubber, false, true);

    // Barrel: collar, rear tube, bore evacuator, front tube
    b.addFrustum(Point3(0.27f, 0.0f, 0.0f), X, 0.3f, 0.21f, 0.17f, 24, palette.hull, true, false);
    b.addFrustum(Point3(0.57f, 0.0f, 0.0f), X, 1.85f, 0.135f, 0.12f, 24, palette.hull, false, false);
    b.addFrustum(Point3(2.42f, 0.0f, 0.0f), X, 0.12f, 0.12f, 0.17f, 24, palette.hull, false, false);
    b.addFrustum(Point3(2.54f, 0.0f, 0.0f), X, 0.5f, 0.17f, 0.17f, 24, palette.hull, false, false);
    b.addFrustum(Point3(3.04f, 0.0f, 0.0f), X, 0.12f, 0.17f, 0.115f, 24, palette.hull, false, false);
    b.addFrustum(Point3(3.16f, 0.0f, 0.0f), X, 1.55f, 0.115f, 0.105f, 24, palette.hull, false, false);

    // Thermal sleeve straps
    const float straps[4] = {1.05f, 1.75f, 3.6f, 4.2f};
    for (int i = 0; i < 4; ++i) {
      float r = (straps[i] < 2.4f) ? 0.142f : 0.122f;
      b.addFrustum(Point3(straps[i], 0.0f, 0.0f), X, 0.035f, r, r, 24, palette.hullDark, true, true);
    }

    // Muzzle reference sensor
    b.addBox(Point3(4.55f, 0.13f, 0.0f), 0.06f, 0.03f, 0.03f, palette.darkMetal);

    // Muzzle brake with side ports and dark bore
    b.addFrustum(Point3(4.71f, 0.0f, 0.0f), X, 0.42f, 0.16f, 0.16f, 24, palette.hullDark, true, true);
    const float portX[2] = {4.8f, 4.97f};
    for (int i = 0; i < 2; ++i) {
      for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
        b.addBox(Point3(portX[i], 0.0f, s * 0.155f), 0.05f, 0.08f, 0.015f, palette.rubber);
      }
    }
    b.addFrustum(Point3(5.13f, 0.0f, 0.0f), X, 0.004f, 0.065f, 0.065f, 16, palette.rubber, false, true);
  }
};

#endif // __TANK_HPP_
