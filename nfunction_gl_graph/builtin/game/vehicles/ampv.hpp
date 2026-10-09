#ifndef __AMPV_HPP_
#define __AMPV_HPP_

#include "armored_vehicle.hpp"
#include "vehicle_parts.hpp"

/**
 * Armored Multi-Purpose Vehicle as drawn on the "Latest U.S. Tanks" poster: an 8x8 wheeled
 * carrier with a V-shaped lower hull, tall box body, rear ramp and a CROWS weapon station.
 *   turret = CROWS base (turret yaw)
 *   gun    = CROWS cradle with the M2 and sensor pod (gun pitch)
 */
class Ampv : public ArmoredVehicle {
public:
  explicit Ampv(float turretYaw = 0.0f, float gunPitch = 0.0f)
    : ArmoredVehicle(turretYaw, gunPitch, TankPalette::fromHullColor(0.30f, 0.35f, 0.23f)) {}

  std::string name() const override { return "AMPV"; }

  Point3 turretMount() const override { return Point3(0.3f, roof(), 0.35f); }
  Point3 gunMount() const override { return vehicle_parts::crowsElevationPivot(); }
  float minGunPitch() const override { return -0.35f; }
  float maxGunPitch() const override { return 1.05f; }

  static float roof() { return 2.1f; }
  static float wheelRadius() { return 0.46f; }
  static float wheelWidth() { return 0.38f; }
  static float wheelZ() { return 1.08f; }

protected:
  void buildHull(TankMeshBuilder &b) const override {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    const float wheelX[4] = {-2.0f, -1.0f, 0.65f, 1.65f};
    const float r = wheelRadius();
    const float top = roof();

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      for (int i = 0; i < 4; ++i) {
        Point3 hub(wheelX[i], r + 0.012f, s * wheelZ());
        addRoadTire(b, p, hub, s, r, wheelWidth());
        b.addRod(Point3(wheelX[i], 0.72f, s * 0.7f), Point3(wheelX[i], r, s * 0.88f), 0.06f, 8, p.darkMetal);
        b.addRod(Point3(wheelX[i] + 0.25f, 0.95f, s * 0.75f), Point3(wheelX[i], r + 0.1f, s * 0.86f), 0.05f, 8,
                 p.steel.shade(0.7f));
      }
    }

    // V-shaped lower hull
    Point3 lower[8] = {
      {-2.65f, 0.38f, -0.32f}, {2.35f, 0.38f, -0.32f}, {2.35f, 0.38f, 0.32f}, {-2.65f, 0.38f, 0.32f},
      {-2.8f, 0.98f, -0.86f}, {3.0f, 0.98f, -0.86f}, {3.0f, 0.98f, 0.86f}, {-2.8f, 0.98f, 0.86f}
    };
    b.addHexahedron(lower, p.hullDark);

    // Box body over the wheels with a long sloped nose
    Point3 body[8] = {
      {-2.8f, 0.98f, -1.3f}, {3.0f, 0.98f, -1.3f}, {3.0f, 0.98f, 1.3f}, {-2.8f, 0.98f, 1.3f},
      {-2.75f, top, -1.2f}, {2.15f, top, -1.2f}, {2.15f, top, 1.2f}, {-2.75f, top, 1.2f}
    };
    b.addHexahedron(body, p.hull);

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      // Wheel arch flares over each axle pair
      b.addBox(Point3(-1.5f, 1.01f, s * 1.3f), 1.0f, 0.04f, 0.06f, p.hullDark);
      b.addBox(Point3(1.15f, 1.01f, s * 1.3f), 1.0f, 0.04f, 0.06f, p.hullDark);

      // Add-on armor panels along the sides
      addArmorTiles(b, p, -2.6f, 0.9f, 1.15f, 1.85f, 1.26f, s, 4, 1, 0.04f, p.hull.shade(0.95f));

      // Vision blocks, headlights, rear lights, stowage
      addVisionBlock(b, p, Point3(-0.4f, 1.95f, s * 1.22f), Point3(0.0f, 0.0f, s));
      addVisionBlock(b, p, Point3(-1.8f, 1.95f, s * 1.22f), Point3(0.0f, 0.0f, s));
      addHeadlight(b, p, Point3(2.95f, 1.15f, s * 1.1f));
      b.addBox(Point3(-2.82f, 1.85f, s * 1.1f), 0.03f, 0.05f, 0.07f, p.tailLight);
      addStowageBox(b, p, Point3(-1.9f, top + 0.1f, s * 0.95f), 0.6f, 0.1f, 0.18f, p.hullDark);

      // Side mirrors on the nose
      b.addRod(Point3(2.4f, 1.85f, s * 1.18f), Point3(2.45f, 2.05f, s * 1.38f), 0.015f, 6, p.darkMetal);
      b.addBox(Point3(2.45f, 2.12f, s * 1.4f), 0.02f, 0.08f, 0.06f, p.darkMetal);

      // Tow eyes
      b.addRod(Point3(2.7f, 0.75f, s * 0.5f), Point3(2.92f, 0.75f, s * 0.5f), 0.05f, 8, p.darkMetal);
      b.addRod(Point3(-2.7f, 0.75f, s * 0.5f), Point3(-2.85f, 0.75f, s * 0.5f), 0.05f, 8, p.darkMetal);
    }

    // Engine grille on the right side of the nose
    b.addBox(Point3(1.55f, 1.55f, 1.26f), 0.4f, 0.2f, 0.01f, p.darkMetal);
    for (int k = 0; k < 6; ++k) {
      b.addBox(Point3(1.55f, 1.4f + 0.06f * k, 1.275f), 0.38f, 0.012f, 0.01f, p.exhaust);
    }

    // Rear ramp with troop door
    b.addBox(Point3(-2.82f, 1.5f, 0.0f), 0.02f, 0.5f, 0.8f, p.hull.shade(0.93f));
    b.addBox(Point3(-2.845f, 1.55f, 0.35f), 0.008f, 0.35f, 0.25f, p.hull.shade(0.88f));
    b.addBox(Point3(-2.855f, 1.55f, 0.15f), 0.01f, 0.03f, 0.03f, p.darkMetal);
    for (int k = -1; k <= 1; k += 2) {
      b.addBox(Point3(-2.84f, 1.05f, k * 0.6f), 0.03f, 0.05f, 0.08f, p.darkMetal);
    }

    // Roof: driver hatch with periscopes, commander and troop hatches, antennas
    addHatch(b, p, Point3(1.85f, top, -0.6f), 0.24f);
    for (int k = -1; k <= 1; ++k) {
      float angle = 0.4f * k;
      Point3 dir(cos(angle), 0.0f, sin(angle));
      addVisionBlock(b, p, Point3(2.08f, top + 0.04f, -0.6f) + dir * 0.06f, dir);
    }
    addHatch(b, p, Point3(0.3f, top, -0.55f), 0.26f);
    addHatch(b, p, Point3(-1.3f, top, 0.0f), 0.3f);
    b.addBox(Point3(-0.6f, top + 0.06f, 0.0f), 0.25f, 0.06f, 0.5f, p.hull.shade(0.95f));
    addAntenna(b, p, Point3(-2.5f, top, 1.0f), 1.4f);
    addAntenna(b, p, Point3(-2.5f, top, -1.0f), 1.2f);
    addSmokeBank(b, p, Point3(1.95f, top - 0.05f, 1.05f), 1.0f, 2, 2);
    addSmokeBank(b, p, Point3(1.95f, top - 0.05f, -1.05f), -1.0f, 2, 2);
  }

  void buildTurret(TankMeshBuilder &b) const override {
    vehicle_parts::addCrowsBase(b, palette);
  }

  void buildGun(TankMeshBuilder &b) const override {
    vehicle_parts::addCrowsCradle(b, palette);
  }
};

#endif // __AMPV_HPP_
