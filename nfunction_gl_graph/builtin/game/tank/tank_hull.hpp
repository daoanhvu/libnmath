#ifndef __TANK_HULL_HPP_
#define __TANK_HULL_HPP_

#include "tank_common.hpp"
#include "tank_running_gear.hpp"

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
    TrackedRunningGear runningGear(TrackedRunningGearSpec(), palette);
    runningGear.build(b, -1.0f);
    runningGear.build(b, 1.0f);
    buildBody(b);
  }

private:
  TankPalette palette;

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
