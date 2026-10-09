#ifndef __TANK_GUN_HPP_
#define __TANK_GUN_HPP_

#include "tank_common.hpp"

/**
 * Main gun: mantlet with coaxial machine gun port, barrel with bore evacuator,
 * thermal sleeve straps, muzzle reference sensor and muzzle brake.
 *
 * Local frame: origin at the trunnion (elevation pivot), barrel along +X.
 * Elevating the gun is a rotation of this frame around +Z.
 */
class TankGun {
public:
  explicit TankGun(const TankPalette &palette = TankPalette::defaultPalette()) : palette(palette) {}

  /** Lower and upper elevation limits in radians (-10 and +20 degrees). */
  static float minPitch() {
    return -0.1745f;
  }

  static float maxPitch() {
    return 0.349f;
  }

  /** Center of the muzzle opening, in gun coordinates. */
  static Point3 muzzle() {
    return Point3(5.13f, 0.0f, 0.0f);
  }

  void build(TankMeshBuilder &b) const {
    const Point3 X(1.0f, 0.0f, 0.0f);

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
    b.addFrustum(muzzle(), X, 0.004f, 0.065f, 0.065f, 16, palette.rubber, false, true);
  }

private:
  TankPalette palette;
};

#endif // __TANK_GUN_HPP_
