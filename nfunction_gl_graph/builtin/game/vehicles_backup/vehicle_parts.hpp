#ifndef __VEHICLE_PARTS_HPP_
#define __VEHICLE_PARTS_HPP_

#include "../tank/tank_common.hpp"

/**
 * Reusable detail parts for armored vehicles. Every function builds in the builder's current
 * frame (+X forward, +Y up, +Z right). Side-dependent parts take s = +1 (right) or -1 (left).
 */
namespace vehicle_parts {

inline Point3 axisX() { return Point3(1.0f, 0.0f, 0.0f); }
inline Point3 axisY() { return Point3(0.0f, 1.0f, 0.0f); }
inline Point3 axisZ() { return Point3(0.0f, 0.0f, 1.0f); }

/** Builds a sub-part through an extra local transform until the scope ends. */
class TransformScope {
public:
  TransformScope(TankMeshBuilder &b, const TankTransform &local) : b(b), saved(b.transform) {
    b.transform = saved * local;
  }
  ~TransformScope() {
    b.transform = saved;
  }

private:
  TankMeshBuilder &b;
  TankTransform saved;
};

/**
 * Main gun along +X from the trunnion: mantlet, collar, tube with optional bore evacuator,
 * thermal sleeve straps, muzzle sensor, optional muzzle brake and a dark bore.
 * The tube radius tapers linearly from tubeRearRadius to tubeFrontRadius.
 */
struct GunSpec {
  float mantletCenterX = 0.05f;
  float mantletHalfX = 0.15f;
  float mantletHalfY = 0.2f;
  float mantletHalfZ = 0.3f;
  bool coaxPort = true;
  float coaxZ = 0.2f;

  float collarStart = 0.2f;
  float collarLength = 0.3f;
  float collarRearRadius = 0.15f;
  float collarFrontRadius = 0.12f;

  float tubeLength = 3.0f;
  float tubeRearRadius = 0.105f;
  float tubeFrontRadius = 0.075f;

  /** Bore evacuator start as a fraction of the tube length, length 0 = none. */
  float evacuatorStart = 0.42f;
  float evacuatorLength = 0.4f;
  float evacuatorRadius = 0.115f;

  int straps = 3;
  bool muzzleSensor = true;

  /** Muzzle brake, length 0 = none. */
  float muzzleBrakeLength = 0.0f;
  float muzzleBrakeRadius = 0.0f;
  int muzzleBrakePorts = 2;

  float boreRadius = 0.05f;

  float tubeStart() const { return collarStart + collarLength; }
  float tubeEnd() const { return tubeStart() + tubeLength; }
  float muzzleX() const { return tubeEnd() + muzzleBrakeLength; }
  float tubeRadiusAt(float x) const {
    return tubeRearRadius + (tubeFrontRadius - tubeRearRadius) * (x - tubeStart()) / tubeLength;
  }
};

inline void addGun(TankMeshBuilder &b, const TankPalette &p, const GunSpec &g) {
  const Point3 X = axisX();
  b.addBox(Point3(g.mantletCenterX, 0.0f, 0.0f), g.mantletHalfX, g.mantletHalfY, g.mantletHalfZ, p.hull);
  if (g.coaxPort) {
    float x = g.mantletCenterX + g.mantletHalfX - 0.01f;
    b.addFrustum(Point3(x, 0.06f, g.coaxZ), X, 0.07f, 0.03f, 0.03f, 10, p.darkMetal, false, true);
    b.addFrustum(Point3(x + 0.071f, 0.06f, g.coaxZ), X, 0.002f, 0.018f, 0.018f, 10, p.rubber, false, true);
  }

  b.addFrustum(Point3(g.collarStart, 0.0f, 0.0f), X, g.collarLength, g.collarRearRadius, g.collarFrontRadius, 24,
               p.hull, true, false);

  const float t0 = g.tubeStart();
  const float t1 = g.tubeEnd();
  const bool openMuzzle = g.muzzleBrakeLength <= 0.0f;
  if (g.evacuatorLength > 0.0f) {
    float e0 = t0 + g.tubeLength * g.evacuatorStart;
    float e1 = e0 + g.evacuatorLength;
    float taper = std::min(0.08f, g.evacuatorLength * 0.25f);
    b.addFrustum(Point3(t0, 0.0f, 0.0f), X, e0 - t0, g.tubeRearRadius, g.tubeRadiusAt(e0), 24, p.hull, false, false);
    b.addFrustum(Point3(e0, 0.0f, 0.0f), X, taper, g.tubeRadiusAt(e0), g.evacuatorRadius, 24, p.hull, false, false);
    b.addFrustum(Point3(e0 + taper, 0.0f, 0.0f), X, e1 - e0 - 2.0f * taper, g.evacuatorRadius, g.evacuatorRadius, 24,
                 p.hull, false, false);
    b.addFrustum(Point3(e1 - taper, 0.0f, 0.0f), X, taper, g.evacuatorRadius, g.tubeRadiusAt(e1), 24, p.hull,
                 false, false);
    b.addFrustum(Point3(e1, 0.0f, 0.0f), X, t1 - e1, g.tubeRadiusAt(e1), g.tubeFrontRadius, 24, p.hull, false,
                 openMuzzle);
  } else {
    b.addFrustum(Point3(t0, 0.0f, 0.0f), X, g.tubeLength, g.tubeRearRadius, g.tubeFrontRadius, 20, p.hull, false,
                 openMuzzle);
  }

  for (int i = 0; i < g.straps; ++i) {
    float x = t0 + g.tubeLength * (static_cast<float>(i) + 0.6f) / static_cast<float>(g.straps + 1);
    if (g.evacuatorLength > 0.0f) {
      float e0 = t0 + g.tubeLength * g.evacuatorStart;
      if (x > e0 - 0.08f && x < e0 + g.evacuatorLength + 0.08f) {
        x = e0 + g.evacuatorLength + 0.25f;
      }
    }
    float r = g.tubeRadiusAt(x) + 0.012f;
    b.addFrustum(Point3(x, 0.0f, 0.0f), X, 0.03f, r, r, 20, p.hullDark, true, true);
  }

  if (g.muzzleSensor) {
    b.addBox(Point3(t1 - 0.12f, g.tubeFrontRadius + 0.022f, 0.0f), 0.05f, 0.022f, 0.025f, p.darkMetal);
  }

  if (!openMuzzle) {
    b.addFrustum(Point3(t1, 0.0f, 0.0f), X, g.muzzleBrakeLength, g.muzzleBrakeRadius, g.muzzleBrakeRadius, 20,
                 p.hullDark, true, true);
    for (int i = 0; i < g.muzzleBrakePorts; ++i) {
      float x = t1 + g.muzzleBrakeLength * (static_cast<float>(i) + 0.5f) / static_cast<float>(g.muzzleBrakePorts);
      float halfX = g.muzzleBrakeLength / (static_cast<float>(g.muzzleBrakePorts) * 3.0f);
      for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
        b.addBox(Point3(x, 0.0f, s * g.muzzleBrakeRadius * 0.97f), halfX, g.muzzleBrakeRadius * 0.55f, 0.012f,
                 p.rubber);
      }
    }
  }
  b.addFrustum(Point3(g.muzzleX() + 0.001f, 0.0f, 0.0f), X, 0.003f, g.boreRadius, g.boreRadius, 16, p.rubber,
               false, true);
}

/** Medium machine gun along +X from its mount point: receiver, barrel, flash hider, ammo box. */
inline void addMachineGun(TankMeshBuilder &b, const TankPalette &p, float barrelLength) {
  const Point3 X = axisX();
  b.addBox(Point3(-0.05f, 0.0f, 0.0f), 0.16f, 0.05f, 0.04f, p.darkMetal);
  b.addRod(Point3(0.1f, 0.015f, 0.0f), Point3(0.1f + barrelLength, 0.015f, 0.0f), 0.018f, 8, p.darkMetal);
  b.addFrustum(Point3(0.1f + barrelLength, 0.015f, 0.0f), X, 0.05f, 0.026f, 0.026f, 8, p.darkMetal, true, true);
  b.addBox(Point3(-0.02f, -0.02f, -0.08f), 0.07f, 0.05f, 0.04f, p.hullDark);
  b.addRod(Point3(-0.21f, 0.0f, -0.035f), Point3(-0.27f, 0.0f, -0.035f), 0.012f, 6, p.darkMetal);
  b.addRod(Point3(-0.21f, 0.0f, 0.035f), Point3(-0.27f, 0.0f, 0.035f), 0.012f, 6, p.darkMetal);
}

/** M2 heavy machine gun along +X: receiver, perforated barrel jacket, long barrel, flash hider. */
inline void addHeavyMachineGun(TankMeshBuilder &b, const TankPalette &p, float barrelLength) {
  const Point3 X = axisX();
  b.addBox(Point3(-0.05f, 0.0f, 0.0f), 0.2f, 0.065f, 0.055f, p.darkMetal);
  b.addFrustum(Point3(0.15f, 0.015f, 0.0f), X, 0.22f, 0.04f, 0.04f, 10, p.darkMetal.shade(0.8f), true, true);
  b.addRod(Point3(0.37f, 0.015f, 0.0f), Point3(0.37f + barrelLength, 0.015f, 0.0f), 0.022f, 8, p.darkMetal);
  b.addFrustum(Point3(0.37f + barrelLength, 0.015f, 0.0f), X, 0.06f, 0.032f, 0.028f, 8, p.darkMetal, true, true);
  b.addRod(Point3(-0.25f, 0.0f, -0.04f), Point3(-0.33f, 0.0f, -0.04f), 0.014f, 6, p.darkMetal);
  b.addRod(Point3(-0.25f, 0.0f, 0.04f), Point3(-0.33f, 0.0f, 0.04f), 0.014f, 6, p.darkMetal);
}

/**
 * Common Remotely Operated Weapon Station, split in two so it can yaw and pitch:
 *   base (yaw part): pedestal, rotating plate and the two cradle arms
 *   cradle (pitch part, origin at crowsElevationPivot()): sensor pod, M2 and ammo can
 */
inline Point3 crowsElevationPivot() {
  return Point3(0.0f, 0.34f, 0.0f);
}

inline void addCrowsBase(TankMeshBuilder &b, const TankPalette &p) {
  b.addFrustum(Point3(0.0f, 0.0f, 0.0f), axisY(), 0.08f, 0.17f, 0.15f, 16, p.darkMetal, false, true);
  b.addBox(Point3(0.0f, 0.12f, 0.0f), 0.2f, 0.04f, 0.2f, p.hull);
  for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
    b.addBox(Point3(-0.02f, 0.26f, s * 0.19f), 0.09f, 0.12f, 0.025f, p.hull);
  }
}

inline void addCrowsCradle(TankMeshBuilder &b, const TankPalette &p) {
  b.addFrustum(Point3(0.0f, 0.0f, -0.2f), axisZ(), 0.4f, 0.04f, 0.04f, 10, p.darkMetal, true, true);
  addHeavyMachineGun(b, p, 0.75f);
  b.addBox(Point3(0.02f, 0.03f, -0.29f), 0.13f, 0.09f, 0.075f, p.hull);
  b.addBox(Point3(0.155f, 0.05f, -0.29f), 0.006f, 0.05f, 0.055f, p.glass);
  b.addBox(Point3(0.155f, -0.02f, -0.27f), 0.006f, 0.02f, 0.02f, p.glass.shade(1.5f));
  b.addBox(Point3(-0.06f, -0.01f, 0.27f), 0.12f, 0.08f, 0.06f, p.hullDark);
}

inline void addCrows(TankMeshBuilder &b, const TankPalette &p, float pitch = 0.0f) {
  addCrowsBase(b, p);
  TransformScope cradle(b, TankTransform::translation(crowsElevationPivot()) * TankTransform::rotationZ(pitch));
  addCrowsCradle(b, p);
}

/**
 * Trophy active protection unit hung on a turret side: support arm, radar box with flat panels
 * and the small launcher turret on top. Origin is the attachment point on the turret wall.
 */
inline void addTrophy(TankMeshBuilder &b, const TankPalette &p, float s) {
  b.addBox(Point3(0.0f, 0.0f, s * 0.07f), 0.12f, 0.05f, 0.07f, p.darkMetal);
  b.addBox(Point3(0.0f, 0.04f, s * 0.24f), 0.26f, 0.16f, 0.11f, p.hullDark);
  b.addBox(Point3(0.0f, 0.04f, s * 0.355f), 0.2f, 0.12f, 0.006f, p.darkMetal);
  b.addBox(Point3(0.265f, 0.04f, s * 0.24f), 0.006f, 0.12f, 0.08f, p.darkMetal);
  b.addFrustum(Point3(-0.05f, 0.2f, s * 0.24f), axisY(), 0.05f, 0.09f, 0.08f, 14, p.hull, false, true);
  b.addBox(Point3(-0.02f, 0.29f, s * 0.24f), 0.12f, 0.04f, 0.06f, p.hull);
}

/**
 * Smoke grenade launcher bank: a base block with cols x rows tubes pointing forward, up and outward.
 */
inline void addSmokeBank(TankMeshBuilder &b, const TankPalette &p, const Point3 &center, float s, int cols, int rows) {
  float halfX = 0.04f + 0.055f * cols;
  float halfY = 0.03f + 0.04f * rows;
  b.addBox(center, halfX, halfY, 0.05f, p.hullDark);
  Point3 dir = Point3(0.5f, 0.45f, s * 0.74f).normalize();
  for (int i = 0; i < cols; ++i) {
    for (int j = 0; j < rows; ++j) {
      Point3 base = center + axisX() * (-0.055f * (cols - 1) + 0.11f * i) +
                    axisY() * (-0.04f * (rows - 1) + 0.08f * j);
      b.addFrustum(base, dir, 0.22f, 0.036f, 0.036f, 8, p.darkMetal, true, false);
      b.addFrustum(base + dir * 0.22f, dir, 0.004f, 0.027f, 0.027f, 8, p.rubber, false, true);
    }
  }
}

/** Round hatch on a flat roof: ring, lid, handle and hinge. center.y is the roof height. */
inline void addHatch(TankMeshBuilder &b, const TankPalette &p, const Point3 &center, float radius) {
  b.addFrustum(center, axisY(), 0.04f, radius, radius * 0.97f, 20, p.hullDark, false, true);
  b.addFrustum(center + axisY() * 0.04f, axisY(), 0.03f, radius * 0.86f, radius * 0.82f, 20, p.hullLight, false, true);
  b.addBox(center + Point3(0.0f, 0.085f, 0.0f), radius * 0.4f, 0.012f, 0.018f, p.darkMetal);
  b.addBox(center + Point3(-radius * 0.85f, 0.06f, 0.0f), 0.03f, 0.03f, radius * 0.45f, p.darkMetal);
}

/** Armored vision block facing the given horizontal direction. */
inline void addVisionBlock(TankMeshBuilder &b, const TankPalette &p, const Point3 &center, const Point3 &outward) {
  Point3 o = outward.normalize();
  Point3 side = axisY().cross(o);
  b.addOrientedBox(center, o, axisY(), side, 0.04f, 0.045f, 0.07f, p.hullDark);
  b.addOrientedBox(center + o * 0.041f, o, axisY(), side, 0.004f, 0.03f, 0.055f, p.glass);
}

/** Box sight head on a short pedestal, glass facing +X. base.y is the roof height. */
inline void addSightHead(TankMeshBuilder &b, const TankPalette &p, const Point3 &base, float pedestal, float half) {
  b.addFrustum(base, axisY(), pedestal, half * 0.7f, half * 0.7f, 14, p.hullDark, false, false);
  Point3 head = base + axisY() * (pedestal + half * 0.75f);
  b.addBox(head, half, half * 0.75f, half, p.hull);
  b.addBox(head + Point3(half + 0.004f, 0.0f, 0.0f), 0.004f, half * 0.45f, half * 0.7f, p.glass);
}

inline void addAntenna(TankMeshBuilder &b, const TankPalette &p, const Point3 &base, float height) {
  b.addFrustum(base, axisY(), 0.07f, 0.055f, 0.045f, 10, p.darkMetal, false, true);
  b.addFrustum(base + axisY() * 0.07f, Point3(-0.1f, 1.0f, 0.0f), height, 0.011f, 0.005f, 6, p.darkMetal, false,
               true);
}

/** Headlight with a guard frame, facing +X. */
inline void addHeadlight(TankMeshBuilder &b, const TankPalette &p, const Point3 &center) {
  const Point3 X = axisX();
  b.addFrustum(center - X * 0.06f, X, 0.1f, 0.065f, 0.07f, 14, p.hullDark, true, false);
  b.addFrustum(center + X * 0.04f, X, 0.006f, 0.058f, 0.058f, 14, p.lens, false, true);
  b.addBox(center + Point3(0.06f, 0.085f, 0.0f), 0.015f, 0.012f, 0.09f, p.darkMetal);
  b.addBox(center + Point3(0.06f, -0.085f, 0.0f), 0.015f, 0.012f, 0.09f, p.darkMetal);
}

/** Engine deck grille: dark frame with raised slats running across the vehicle. */
inline void addGrille(TankMeshBuilder &b, const TankPalette &p, const Point3 &center, float halfX, float halfZ,
                      int slats) {
  b.addBox(center, halfX, 0.012f, halfZ, p.darkMetal);
  float step = (2.0f * halfX) / static_cast<float>(slats);
  for (int k = 0; k < slats; ++k) {
    float x = center.x - halfX + step * (static_cast<float>(k) + 0.5f);
    b.addBox(Point3(x, center.y + 0.02f, center.z), step * 0.28f, 0.014f, halfZ * 0.95f, p.hullDark);
  }
}

/** Stowage box with a slightly wider lid and two latches on the +X face. */
inline void addStowageBox(TankMeshBuilder &b, const TankPalette &p, const Point3 &center, float hx, float hy, float hz,
                          const TankColor &color) {
  b.addBox(center, hx, hy, hz, color);
  b.addBox(center + Point3(0.0f, hy + 0.012f, 0.0f), hx + 0.012f, 0.012f, hz + 0.012f, color.shade(1.1f));
  for (float k = -1.0f; k <= 1.0f; k += 2.0f) {
    b.addBox(center + Point3(k * hx * 0.6f, hy * 0.6f, 0.0f), 0.025f, 0.03f, hz + 0.008f, p.darkMetal);
  }
}

/**
 * Side skirt panels between x0 and x1 on one side. The front panel's bottom edge is raked back.
 * Z values are magnitudes, s picks the side.
 */
inline void addSkirtPanels(TankMeshBuilder &b, const TankPalette &p, float x0, float x1, float yBottom, float yTop,
                           float zIn, float zOut, float s, int panels, float frontRake) {
  const float gap = 0.022f;
  float panelLength = ((x1 - x0) - gap * (panels - 1)) / static_cast<float>(panels);
  for (int i = 0; i < panels; ++i) {
    float a = x0 + i * (panelLength + gap);
    float c = a + panelLength;
    float bottomC = (i == panels - 1) ? c - frontRake : c;
    Point3 panel[8] = {
      {a, yBottom, s * zIn}, {bottomC, yBottom, s * zIn}, {bottomC, yBottom, s * zOut}, {a, yBottom, s * zOut},
      {a, yTop, s * zIn}, {c, yTop, s * zIn}, {c, yTop, s * zOut}, {a, yTop, s * zOut}
    };
    b.addHexahedron(panel, (i % 2 == 0) ? p.hull : p.hull.shade(0.94f));
    b.addBox(Point3((a + bottomC) * 0.5f, yBottom - 0.015f, s * (zIn + zOut) * 0.5f), (bottomC - a) * 0.5f, 0.015f,
             (zOut - zIn) * 0.5f, p.rubber);
    for (int k = 0; k < 2; ++k) {
      float bx = a + panelLength * (k == 0 ? 0.2f : 0.8f);
      b.addBox(Point3(bx, yTop - 0.06f, s * (zOut + 0.006f)), 0.015f, 0.015f, 0.006f, p.darkMetal);
    }
  }
}

/**
 * Grid of reactive armor tiles on a vertical side face at |z| = zFace, protruding outward by depth.
 */
inline void addArmorTiles(TankMeshBuilder &b, const TankPalette &p, float x0, float x1, float y0, float y1, float zFace,
                          float s, int cols, int rows, float depth, const TankColor &color) {
  const float gap = 0.025f;
  float w = ((x1 - x0) - gap * (cols - 1)) / static_cast<float>(cols);
  float h = ((y1 - y0) - gap * (rows - 1)) / static_cast<float>(rows);
  for (int i = 0; i < cols; ++i) {
    for (int j = 0; j < rows; ++j) {
      float cx = x0 + w * 0.5f + i * (w + gap);
      float cy = y0 + h * 0.5f + j * (h + gap);
      b.addBox(Point3(cx, cy, s * (zFace + depth * 0.5f)), w * 0.5f, h * 0.5f, depth * 0.5f,
               ((i + j) % 2 == 0) ? color : color.shade(0.93f));
      b.addBox(Point3(cx, cy, s * (zFace + depth + 0.006f)), 0.018f, 0.018f, 0.006f, p.darkMetal);
    }
  }
}

/**
 * Open basket made of rods behind a turret bustle, from xFront back to xBack, |z| <= halfZ.
 */
inline void addBasket(TankMeshBuilder &b, const TankPalette &p, float xFront, float xBack, float halfZ, float yBottom,
                      float yTop) {
  const float r = 0.018f;
  const float levels[2] = {yBottom, yTop};
  for (int i = 0; i < 2; ++i) {
    float y = levels[i];
    b.addRod(Point3(xFront, y, -halfZ), Point3(xBack, y, -halfZ), r, 6, p.darkMetal);
    b.addRod(Point3(xFront, y, halfZ), Point3(xBack, y, halfZ), r, 6, p.darkMetal);
    b.addRod(Point3(xBack, y, -halfZ), Point3(xBack, y, halfZ), r, 6, p.darkMetal);
  }
  for (int k = 0; k < 5; ++k) {
    float z = -halfZ + 2.0f * halfZ * static_cast<float>(k) / 4.0f;
    b.addRod(Point3(xBack, yBottom, z), Point3(xBack, yTop, z), r, 6, p.darkMetal);
  }
  for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
    b.addRod(Point3((xFront + xBack) * 0.5f, yBottom, s * halfZ), Point3((xFront + xBack) * 0.5f, yTop, s * halfZ), r,
             6, p.darkMetal);
  }
  for (int k = 1; k < 3; ++k) {
    float x = xFront + (xBack - xFront) * static_cast<float>(k) / 3.0f;
    b.addRod(Point3(x, yBottom, -halfZ), Point3(x, yBottom, halfZ), r, 6, p.darkMetal);
  }
}

/** Rolled tarp along Z with two straps. */
inline void addTarpRoll(TankMeshBuilder &b, const TankPalette &p, const Point3 &center, float radius, float length) {
  Point3 base = center - axisZ() * (length * 0.5f);
  b.addFrustum(base, axisZ(), length, radius, radius, 16, p.canvas, true, true);
  for (int k = 0; k < 2; ++k) {
    b.addFrustum(base + axisZ() * (length * (k == 0 ? 0.22f : 0.75f)), axisZ(), 0.035f, radius * 1.05f,
                 radius * 1.05f, 16, p.canvas.shade(0.6f), true, true);
  }
}

/**
 * Wheel for wheeled vehicles: tire with tread lugs, painted rim, hub and lug nuts.
 * center is the axle center, s picks the outward side.
 */
inline void addRoadTire(TankMeshBuilder &b, const TankPalette &p, const Point3 &center, float s, float radius,
                        float width) {
  Point3 out(0.0f, 0.0f, s);
  Point3 inner = center - out * (width * 0.5f);
  b.addFrustum(inner, out, width, radius - 0.02f, radius - 0.02f, 28, p.rubber, true, true);
  const int lugs = 22;
  for (int k = 0; k < lugs; ++k) {
    float angle = 2.0f * M_PI * static_cast<float>(k) / static_cast<float>(lugs);
    Point3 radial(cos(angle), sin(angle), 0.0f);
    Point3 tangent(-sin(angle), cos(angle), 0.0f);
    float offset = (k % 2 == 0) ? -0.18f : 0.18f;
    b.addOrientedBox(center + radial * (radius - 0.01f) + out * (width * offset), radial, tangent, out,
                     0.02f, 0.045f, width * 0.3f, p.rubber.shade(1.3f));
  }
  Point3 face = center + out * (width * 0.5f);
  b.addFrustum(face, out, 0.015f, radius * 0.62f, radius * 0.6f, 24, p.hull, false, true);
  b.addFrustum(face + out * 0.015f, out, 0.05f, radius * 0.28f, radius * 0.24f, 16, p.darkMetal, false, true);
  b.addFrustum(face + out * 0.065f, out, 0.04f, radius * 0.1f, radius * 0.08f, 10, p.steel, false, true);
  for (int k = 0; k < 8; ++k) {
    float angle = 2.0f * M_PI * static_cast<float>(k) / 8.0f;
    Point3 nut = face + out * 0.015f + Point3(cos(angle), sin(angle), 0.0f) * (radius * 0.42f);
    b.addFrustum(nut, out, 0.02f, 0.016f, 0.016f, 6, p.steel.shade(0.8f), false, true);
  }
  b.addFrustum(center - out * (width * 0.5f), out * -1.0f, 0.01f, radius * 0.6f, radius * 0.6f, 20, p.darkMetal,
               false, true);
}

} // namespace vehicle_parts

#endif // __VEHICLE_PARTS_HPP_
