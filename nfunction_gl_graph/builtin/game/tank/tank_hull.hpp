#ifndef __TANK_HULL_HPP_
#define __TANK_HULL_HPP_

#include "tank_common.hpp"

/**
 * Hull with running gear: tracks, road wheels, idler, sprocket, return rollers,
 * armor body and deck details.
 *
 * Local frame: origin at the center of the tank on the ground plane, the tracks sit on y = 0.
 */
class TankHull {
public:
  explicit TankHull(const TankPalette &palette = TankPalette::defaultPalette()) : palette(palette) {}

  /** Center of the turret ring on the top deck, in hull coordinates. */
  static Point3 turretMount() {
    return Point3(-0.25f, 1.78f, 0.0f);
  }

  void build(TankMeshBuilder &b) const {
    buildRunningGear(b, -1.0f);
    buildRunningGear(b, 1.0f);
    buildBody(b);
  }

private:
  static constexpr float grouserHeight = 0.04f;

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

  void buildBody(TankMeshBuilder &b) const {
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
};

#endif // __TANK_HULL_HPP_
