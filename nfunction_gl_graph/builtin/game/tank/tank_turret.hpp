#ifndef __TANK_TURRET_HPP_
#define __TANK_TURRET_HPP_

#include "tank_common.hpp"

/**
 * Turret without the gun: ring, armored body and front wedge, bustle with rack,
 * cupola, hatches, machine gun, sights, smoke launchers and antennas.
 *
 * Local frame: origin at the center of the turret ring (bottom), front of the turret along +X.
 * Rotating the turret is a rotation of this frame around +Y.
 */
class TankTurret {
public:
  explicit TankTurret(const TankPalette &palette = TankPalette::defaultPalette()) : palette(palette) {}

  /** Gun trunnion (pivot of the elevation axis), in turret coordinates. */
  static Point3 gunMount() {
    return Point3(1.72f, 0.4f, 0.0f);
  }

  void build(TankMeshBuilder &b) const {
    const Point3 X(1.0f, 0.0f, 0.0f);
    const Point3 Y(0.0f, 1.0f, 0.0f);
    const Point3 Z(0.0f, 0.0f, 1.0f);

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
  }

private:
  TankPalette palette;
};

#endif // __TANK_TURRET_HPP_
