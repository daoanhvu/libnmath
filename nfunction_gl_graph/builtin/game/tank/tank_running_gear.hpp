#ifndef __TANK_RUNNING_GEAR_HPP_
#define __TANK_RUNNING_GEAR_HPP_

#include "tank_common.hpp"

struct TrackWheelSpec {
  float x;
  float y;
  float r;
};

/**
 * Layout of one side of a tracked running gear. Distances are in vehicle units, Z values are
 * magnitudes (the side sign is applied when building). Defaults describe the generic Tank.
 */
struct TrackedRunningGearSpec {
  float trackCenterZ = 1.5f;
  float trackWidth = 0.62f;
  float trackThickness = 0.08f;
  float grouserHeight = 0.04f;
  float shoeSpacing = 0.2f;

  float roadWheelRadius = 0.36f;
  float roadWheelGap = 0.04f;
  float roadWheelTireWidth = 0.22f;
  /** Road wheel centers from rear to front. */
  std::vector<float> roadWheelX = {-2.1f, -1.26f, -0.42f, 0.42f, 1.26f, 2.1f};

  /** Drive sprocket and idler; whichever has the larger x is the front wheel of the belt. */
  TrackWheelSpec sprocket = {-2.85f, 0.78f, 0.34f};
  TrackWheelSpec idler = {2.9f, 0.74f, 0.30f};
  float sprocketRingOffset = 0.15f;
  float sprocketHubHalfLength = 0.22f;
  float sprocketCapOffset = 0.19f;
  int sprocketTeeth = 11;

  std::vector<float> returnRollerX = {-1.4f, 0.0f, 1.4f};
  float returnRollerRadius = 0.1f;
  /** Distance from the inner surface of the top run down to the roller axle. */
  float returnRollerDrop = 0.105f;
  float returnRollerOffset = 0.13f;
  float returnRollerWidth = 0.12f;
  float returnRollerAxleEnd = 0.19f;

  /** Suspension arms run from the hull side wall to each road wheel axle. */
  float hullSideZ = 1.12f;
  float suspensionArmY = 0.62f;
  float suspensionArmLength = 0.32f;
  float suspensionArmEndZ = 1.25f;

  float arcStep = 0.17f;
  float lineStep = 0.25f;

  float roadWheelY() const {
    return grouserHeight + trackThickness + roadWheelRadius;
  }
};

/**
 * Tracks, road wheels, idler, sprocket, return rollers and suspension arms.
 */
class TrackedRunningGear {
public:
  TrackedRunningGear(const TrackedRunningGearSpec &spec, const TankPalette &palette) : spec(spec), palette(palette) {}

  /** Param s: +1 for the right side (+Z), -1 for the left side */
  void build(TankMeshBuilder &b, float s) const {
    const float zc = s * spec.trackCenterZ;
    const float wheelY = spec.roadWheelY();
    const TrackWheelSpec &front = (spec.idler.x > spec.sprocket.x) ? spec.idler : spec.sprocket;
    const TrackWheelSpec &rear = (spec.idler.x > spec.sprocket.x) ? spec.sprocket : spec.idler;

    for (size_t i = 0; i < spec.roadWheelX.size(); ++i) {
      float x = spec.roadWheelX[i];
      addRoadWheel(b, x, wheelY, zc, s, spec.roadWheelRadius);
      b.addRod(Point3(x + spec.suspensionArmLength, spec.suspensionArmY, s * spec.hullSideZ),
               Point3(x, wheelY, s * spec.suspensionArmEndZ), 0.045f, 8, palette.darkMetal);
    }
    addRoadWheel(b, spec.idler.x, spec.idler.y, zc, s, spec.idler.r);
    addSprocket(b, spec.sprocket.x, spec.sprocket.y, zc, s, spec.sprocket.r);

    for (size_t i = 0; i < spec.returnRollerX.size(); ++i) {
      float x = spec.returnRollerX[i];
      float topRunY = front.y + front.r + (front.x - x) / (front.x - rear.x) *
                      ((rear.y + rear.r) - (front.y + front.r));
      float y = topRunY - spec.returnRollerDrop;
      for (int half = 0; half < 2; ++half) {
        float offset = (half == 0) ? -spec.returnRollerOffset : spec.returnRollerOffset;
        b.addFrustum(Point3(x, y, zc + offset - spec.returnRollerWidth * 0.5f), Point3(0.0f, 0.0f, 1.0f),
                     spec.returnRollerWidth, spec.returnRollerRadius, spec.returnRollerRadius, 16,
                     palette.rubber, true, true);
      }
      b.addRod(Point3(x, y, s * spec.hullSideZ), Point3(x, y, zc + s * spec.returnRollerAxleEnd), 0.04f, 8,
               palette.darkMetal);
    }

    std::vector<TrackWheel> wheels = {
      {spec.roadWheelX.front(), wheelY, spec.roadWheelRadius},
      {spec.roadWheelX.back(), wheelY, spec.roadWheelRadius},
      {front.x, front.y, front.r},
      {rear.x, rear.y, rear.r}
    };
    std::vector<TrackSample> path = buildTrackPath(wheels, spec.arcStep, spec.lineStep);
    addTrackBand(b, path, zc);
    addTrackShoes(b, path, zc);
  }

private:
  TrackedRunningGearSpec spec;
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

  void addTrackBand(TankMeshBuilder &b, const std::vector<TrackSample> &path, float zc) const {
    const float width = spec.trackWidth;
    const float thickness = spec.trackThickness;
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
  void addTrackShoes(TankMeshBuilder &b, const std::vector<TrackSample> &path, float zc) const {
    const float width = spec.trackWidth;
    const float thickness = spec.trackThickness;
    const float grouserHeight = spec.grouserHeight;
    size_t n = path.size();
    std::vector<float> cumulative(n + 1, 0.0f);
    for (size_t i = 0; i < n; ++i) {
      const TrackSample &p = path[i];
      const TrackSample &q = path[(i + 1) % n];
      cumulative[i + 1] = cumulative[i] + sqrt((q.x - p.x) * (q.x - p.x) + (q.y - p.y) * (q.y - p.y));
    }
    float total = cumulative[n];
    int shoes = std::max(1, static_cast<int>(round(total / spec.shoeSpacing)));
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
    const float gap = spec.roadWheelGap;
    const float tire = spec.roadWheelTireWidth;
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
   * Drive sprocket: axle hub and two toothed rings that straddle the guide horns.
   */
  void addSprocket(TankMeshBuilder &b, float cx, float cy, float zc, float s, float radius) const {
    Point3 out(0.0f, 0.0f, s);
    Point3 axisZ(0.0f, 0.0f, 1.0f);
    Point3 c(cx, cy, zc);
    const int teeth = spec.sprocketTeeth;

    b.addFrustum(c - out * spec.sprocketHubHalfLength, out, spec.sprocketHubHalfLength * 2.0f, 0.14f, 0.14f, 16,
                 palette.darkMetal, true, true);
    for (int ring = 0; ring < 2; ++ring) {
      Point3 ringCenter = c + out * (ring == 0 ? -spec.sprocketRingOffset : spec.sprocketRingOffset);
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
    b.addFrustum(c + out * spec.sprocketCapOffset, out, 0.07f, 0.13f, 0.09f, 16, palette.steel, false, true);
    for (int k = 0; k < 8; ++k) {
      float angle = 2.0f * M_PI * static_cast<float>(k) / 8.0f;
      Point3 p = c + Point3(cos(angle) * 0.2f, sin(angle) * 0.2f, 0.0f) + out * spec.sprocketCapOffset;
      b.addFrustum(p, out, 0.025f, 0.018f, 0.018f, 6, palette.steel.shade(0.8f), false, true);
    }
  }
};

#endif // __TANK_RUNNING_GEAR_HPP_
