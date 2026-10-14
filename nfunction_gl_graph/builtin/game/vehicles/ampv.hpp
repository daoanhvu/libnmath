#ifndef __AMPV_HPP_
#define __AMPV_HPP_

#include <string>
#include "armored_vehicle.hpp"
#include "vehicle_parts.hpp"

/**
 * AMPV assembled from replaceable parts:
 *   hull:   eight road wheels (wheel_l0 .. wheel_r3) and the hull body
 *   turret: CROWS base (turret yaw)
 *   gun:    CROWS cradle with the M2 and sensor pod (gun pitch)
 * Swap "turret" or "gun" to change the weapon station. Swap a wheel_* part to change a wheel.
 */
class Ampv : public ArmoredVehicle {
public:
  explicit Ampv(float turretYaw = 0.0f, float gunPitch = 0.0f)
    : ArmoredVehicle("AMPV", turretYaw, gunPitch, TankPalette::fromHullColor(0.30f, 0.35f, 0.23f)) {
    const TankPalette p = palette;
    setTurretMount(Point3(0.3f, roof(), 0.35f));
    setGunMount(vehicle_parts::crowsElevationPivot());
    setGunPitchLimits(-0.35f, 1.05f);

    std::unique_ptr<ComplexVehiclePart> hull(new ComplexVehiclePart("hull"));
    const float wheelX[4] = {-2.0f, -1.0f, 0.65f, 1.65f};
    const float radius = wheelRadius();
    const float width = wheelWidth();
    const float z = wheelZ();
    const float sides[2] = {-1.0f, 1.0f};
    for (int sideIndex = 0; sideIndex < 2; ++sideIndex) {
      const float s = sides[sideIndex];
      for (int i = 0; i < 4; ++i) {
        const float x = wheelX[i];
        const std::string partName = std::string("wheel_") + (s < 0.0f ? "l" : "r") + std::to_string(i);
        hull->add(makeMeshPart(partName, [p, s, x, radius, width, z](TankMeshBuilder &b) {
          using namespace vehicle_parts;
          Point3 hub(x, radius + 0.012f, s * z);
          addRoadTire(b, p, hub, s, radius, width);
          b.addRod(Point3(x, 0.72f, s * 0.7f), Point3(x, radius, s * 0.88f), 0.06f, 8, p.darkMetal);
          b.addRod(Point3(x + 0.25f, 0.95f, s * 0.75f), Point3(x, radius + 0.1f, s * 0.86f), 0.05f, 8,
                   p.steel.shade(0.7f));
        }));
      }
    }
    hull->add(makeMeshPart("hull_body", [p](TankMeshBuilder &b) { buildHullBody(b, p); }));
    add(std::move(hull));
    add(std::unique_ptr<VehiclePart>(new CrowsBasePart("turret", p)));
    add(std::unique_ptr<VehiclePart>(new CrowsCradlePart("gun", p)));
  }

  static float roof() { return 2.1f; }
  static float wheelRadius() { return 0.46f; }
  static float wheelWidth() { return 0.38f; }
  static float wheelZ() { return 1.08f; }

private:
  static void buildHullBody(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const float top = roof();
    Point3 lower[8] = {
      {-2.65f, 0.38f, -0.32f}, {2.35f, 0.38f, -0.32f}, {2.35f, 0.38f, 0.32f}, {-2.65f, 0.38f, 0.32f},
      {-2.8f, 0.98f, -0.86f}, {3.0f, 0.98f, -0.86f}, {3.0f, 0.98f, 0.86f}, {-2.8f, 0.98f, 0.86f}
    };
    b.addHexahedron(lower, p.hullDark);
    Point3 body[8] = {
      {-2.8f, 0.98f, -1.3f}, {3.0f, 0.98f, -1.3f}, {3.0f, 0.98f, 1.3f}, {-2.8f, 0.98f, 1.3f},
      {-2.75f, top, -1.2f}, {2.15f, top, -1.2f}, {2.15f, top, 1.2f}, {-2.75f, top, 1.2f}
    };
    b.addHexahedron(body, p.hull);

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      b.addBox(Point3(-1.5f, 1.01f, s * 1.3f), 1.0f, 0.04f, 0.06f, p.hullDark);
      b.addBox(Point3(1.15f, 1.01f, s * 1.3f), 1.0f, 0.04f, 0.06f, p.hullDark);
      addArmorTiles(b, p, -2.6f, 0.9f, 1.15f, 1.85f, 1.26f, s, 4, 1, 0.04f, p.hull.shade(0.95f));
      addVisionBlock(b, p, Point3(-0.4f, 1.95f, s * 1.22f), Point3(0.0f, 0.0f, s));
      addVisionBlock(b, p, Point3(-1.8f, 1.95f, s * 1.22f), Point3(0.0f, 0.0f, s));
      addHeadlight(b, p, Point3(2.95f, 1.15f, s * 1.1f));
      b.addBox(Point3(-2.82f, 1.85f, s * 1.1f), 0.03f, 0.05f, 0.07f, p.tailLight);
      addStowageBox(b, p, Point3(-1.9f, top + 0.1f, s * 0.95f), 0.6f, 0.1f, 0.18f, p.hullDark);
      b.addRod(Point3(2.4f, 1.85f, s * 1.18f), Point3(2.45f, 2.05f, s * 1.38f), 0.015f, 6, p.darkMetal);
      b.addBox(Point3(2.45f, 2.12f, s * 1.4f), 0.02f, 0.08f, 0.06f, p.darkMetal);
      b.addRod(Point3(2.7f, 0.75f, s * 0.5f), Point3(2.92f, 0.75f, s * 0.5f), 0.05f, 8, p.darkMetal);
      b.addRod(Point3(-2.7f, 0.75f, s * 0.5f), Point3(-2.85f, 0.75f, s * 0.5f), 0.05f, 8, p.darkMetal);
    }

    b.addBox(Point3(1.55f, 1.55f, 1.26f), 0.4f, 0.2f, 0.01f, p.darkMetal);
    for (int k = 0; k < 6; ++k) {
      b.addBox(Point3(1.55f, 1.4f + 0.06f * k, 1.275f), 0.38f, 0.012f, 0.01f, p.exhaust);
    }
    b.addBox(Point3(-2.82f, 1.5f, 0.0f), 0.02f, 0.5f, 0.8f, p.hull.shade(0.93f));
    b.addBox(Point3(-2.845f, 1.55f, 0.35f), 0.008f, 0.35f, 0.25f, p.hull.shade(0.88f));
    b.addBox(Point3(-2.855f, 1.55f, 0.15f), 0.01f, 0.03f, 0.03f, p.darkMetal);
    for (int k = -1; k <= 1; k += 2) {
      b.addBox(Point3(-2.84f, 1.05f, k * 0.6f), 0.03f, 0.05f, 0.08f, p.darkMetal);
    }
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
};

#endif // __AMPV_HPP_
