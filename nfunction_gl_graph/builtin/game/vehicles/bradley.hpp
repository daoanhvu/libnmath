#ifndef __BRADLEY_HPP_
#define __BRADLEY_HPP_

#include "armored_vehicle.hpp"
#include "vehicle_parts.hpp"
#include "../tank/tank_running_gear.hpp"

/**
 * M3A3E1 Bradley cavalry fighting vehicle: front engine and drive sprocket, six road wheels,
 * BRAT reactive armor tiles on the hull sides, trim vane on the bow, rear ramp.
 * Two-man turret slightly offset to the right with a TOW launcher on the left side,
 * ISU sight, commander's independent viewer and the 25 mm M242 Bushmaster.
 */
class Bradley : public ArmoredVehicle {
public:
  explicit Bradley(float turretYaw = 0.0f, float gunPitch = 0.0f)
    : ArmoredVehicle(turretYaw, gunPitch, TankPalette::fromHullColor(0.30f, 0.35f, 0.23f)) {}

  std::string name() const override { return "M3A3E1 Bradley"; }

  Point3 turretMount() const override { return Point3(-0.35f, hullTop(), 0.15f); }
  Point3 gunMount() const override { return Point3(1.1f, 0.3f, -0.05f); }
  float minGunPitch() const override { return -0.17f; }
  float maxGunPitch() const override { return 1.05f; }

  static float hullTop() { return 1.7f; }

  static TrackedRunningGearSpec runningGear() {
    TrackedRunningGearSpec g;
    g.trackCenterZ = 1.15f;
    g.trackWidth = 0.46f;
    g.trackThickness = 0.06f;
    g.grouserHeight = 0.035f;
    g.shoeSpacing = 0.15f;
    g.roadWheelRadius = 0.25f;
    g.roadWheelGap = 0.035f;
    g.roadWheelTireWidth = 0.16f;
    g.roadWheelX = {-1.75f, -1.05f, -0.35f, 0.35f, 1.05f, 1.75f};
    g.sprocket = {2.32f, 0.6f, 0.24f};
    g.idler = {-2.4f, 0.48f, 0.23f};
    g.sprocketRingOffset = 0.1f;
    g.sprocketHubHalfLength = 0.17f;
    g.sprocketCapOffset = 0.14f;
    g.sprocketTeeth = 10;
    g.returnRollerX = {-1.2f, 0.0f, 1.2f};
    g.returnRollerRadius = 0.07f;
    g.returnRollerDrop = 0.075f;
    g.returnRollerOffset = 0.09f;
    g.returnRollerWidth = 0.09f;
    g.returnRollerAxleEnd = 0.15f;
    g.hullSideZ = 0.86f;
    g.suspensionArmY = 0.46f;
    g.suspensionArmLength = 0.26f;
    g.suspensionArmEndZ = 0.94f;
    return g;
  }

  static vehicle_parts::GunSpec gunSpec() {
    vehicle_parts::GunSpec g;
    g.mantletCenterX = 0.0f;
    g.mantletHalfX = 0.18f;
    g.mantletHalfY = 0.15f;
    g.mantletHalfZ = 0.22f;
    g.coaxZ = 0.15f;
    g.collarStart = 0.15f;
    g.collarLength = 0.25f;
    g.collarRearRadius = 0.075f;
    g.collarFrontRadius = 0.065f;
    g.tubeLength = 1.6f;
    g.tubeRearRadius = 0.04f;
    g.tubeFrontRadius = 0.034f;
    g.evacuatorLength = 0.0f;
    g.straps = 0;
    g.muzzleSensor = false;
    g.muzzleBrakeLength = 0.12f;
    g.muzzleBrakeRadius = 0.055f;
    g.muzzleBrakePorts = 1;
    g.boreRadius = 0.018f;
    return g;
  }

protected:
  void buildHull(TankMeshBuilder &b) const override {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    const float top = hullTop();
    TrackedRunningGear gear(runningGear(), p);
    gear.build(b, 1.0f);
    gear.build(b, -1.0f);

    Point3 lower[8] = {
      {-2.45f, 0.4f, -0.86f}, {2.3f, 0.4f, -0.86f}, {2.3f, 0.4f, 0.86f}, {-2.45f, 0.4f, 0.86f},
      {-2.55f, 0.95f, -0.86f}, {2.75f, 0.95f, -0.86f}, {2.75f, 0.95f, 0.86f}, {-2.55f, 0.95f, 0.86f}
    };
    b.addHexahedron(lower, p.hullDark);

    // Upper hull with the long sloped glacis
    Point3 upper[8] = {
      {-2.55f, 0.95f, -1.4f}, {2.75f, 0.95f, -1.4f}, {2.75f, 0.95f, 1.4f}, {-2.55f, 0.95f, 1.4f},
      {-2.55f, top, -1.32f}, {1.55f, top, -1.32f}, {1.55f, top, 1.32f}, {-2.55f, top, 1.32f}
    };
    b.addHexahedron(upper, p.hull);

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      addSkirtPanels(b, p, -2.45f, 2.6f, 0.55f, 0.97f, 1.4f, 1.45f, s, 5, 0.22f);

      // BRAT reactive armor tiles, two rows along the side
      addArmorTiles(b, p, -2.3f, 2.2f, 0.62f, 1.5f, 1.4f, s, 7, 2, 0.1f, p.hull.shade(0.92f));

      addHeadlight(b, p, Point3(2.45f, 1.18f, s * 1.18f));
      b.addBox(Point3(-2.57f, 1.45f, s * 1.15f), 0.03f, 0.05f, 0.07f, p.tailLight);
      b.addRod(Point3(2.45f, 0.65f, s * 0.55f), Point3(2.65f, 0.65f, s * 0.55f), 0.05f, 8, p.darkMetal);
      b.addRod(Point3(-2.45f, 0.65f, s * 0.55f), Point3(-2.6f, 0.65f, s * 0.55f), 0.05f, 8, p.darkMetal);
      addStowageBox(b, p, Point3(-1.85f, top + 0.1f, s * 1.08f), 0.55f, 0.1f, 0.16f, p.hullDark);
    }

    // Trim vane folded flat on the glacis
    Point3 glacisDir = Point3(-1.2f, 0.75f, 0.0f).normalize();
    Point3 glacisNormal = Point3(0.75f, 1.2f, 0.0f).normalize();
    Point3 vaneCenter = Point3(2.3f, 1.23f, 0.0f) + glacisNormal * 0.03f;
    b.addOrientedBox(vaneCenter, glacisDir, glacisNormal, axisZ(), 0.35f, 0.025f, 1.2f, p.hull.shade(1.08f));
    b.addOrientedBox(vaneCenter + glacisNormal * 0.03f - glacisDir * 0.3f, glacisDir, glacisNormal, axisZ(), 0.02f,
                     0.01f, 1.1f, p.darkMetal);

    // Engine grille on the front right, exhaust outlet
    addGrille(b, p, Point3(1.0f, top + 0.005f, 0.85f), 0.35f, 0.32f, 6);
    b.addFrustum(Point3(1.4f, top, 1.05f), axisY(), 0.12f, 0.07f, 0.07f, 10, p.exhaust, false, true);

    // Driver hatch with periscopes, front left
    addHatch(b, p, Point3(1.25f, top, -0.75f), 0.23f);
    for (int k = -1; k <= 1; ++k) {
      float angle = 0.4f * k;
      Point3 dir(cos(angle), 0.0f, sin(angle));
      addVisionBlock(b, p, Point3(1.5f, top + 0.04f, -0.75f) + dir * 0.06f, dir);
    }

    // Rear ramp, troop door and cargo hatch
    b.addBox(Point3(-2.57f, 1.25f, 0.0f), 0.02f, 0.4f, 0.75f, p.hull.shade(0.93f));
    b.addBox(Point3(-2.595f, 1.3f, -0.3f), 0.008f, 0.3f, 0.22f, p.hull.shade(0.88f));
    b.addBox(Point3(-1.9f, top + 0.015f, 0.0f), 0.45f, 0.015f, 0.55f, p.hull.shade(0.95f));
    addAntenna(b, p, Point3(-2.35f, top, 1.15f), 1.3f);
    addAntenna(b, p, Point3(-2.35f, top, -1.15f), 1.1f);
  }

  void buildTurret(TankMeshBuilder &b) const override {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    const Point3 Y = axisY();
    const float roof = 0.62f;

    b.addFrustum(Point3(0.0f, 0.0f, 0.0f), Y, 0.06f, 0.75f, 0.75f, 28, p.darkMetal, false, true);

    Point3 body[8] = {
      {-0.95f, 0.05f, -0.85f}, {0.9f, 0.05f, -0.85f}, {0.9f, 0.05f, 0.85f}, {-0.95f, 0.05f, 0.85f},
      {-0.9f, roof, -0.72f}, {0.7f, roof, -0.72f}, {0.7f, roof, 0.72f}, {-0.9f, roof, 0.72f}
    };
    b.addHexahedron(body, p.hull);
    Point3 front[8] = {
      {0.9f, 0.05f, -0.85f}, {1.15f, 0.05f, -0.45f}, {1.15f, 0.05f, 0.45f}, {0.9f, 0.05f, 0.85f},
      {0.7f, roof, -0.72f}, {0.95f, roof, -0.4f}, {0.95f, roof, 0.4f}, {0.7f, roof, 0.72f}
    };
    b.addHexahedron(front, p.hull.shade(1.05f));
    Point3 bustle[8] = {
      {-1.4f, 0.12f, -0.7f}, {-0.9f, 0.12f, -0.7f}, {-0.9f, 0.12f, 0.7f}, {-1.4f, 0.12f, 0.7f},
      {-1.45f, 0.58f, -0.66f}, {-0.9f, 0.58f, -0.66f}, {-0.9f, 0.58f, 0.66f}, {-1.45f, 0.58f, 0.66f}
    };
    b.addHexahedron(bustle, p.hull);
    addBasket(b, p, -1.43f, -1.75f, 0.62f, 0.2f, 0.52f);
    addTarpRoll(b, p, Point3(-1.6f, 0.32f, 0.0f), 0.1f, 1.1f);

    // TOW launcher on the left side: twin tubes on a raising arm
    b.addBox(Point3(-0.25f, 0.4f, -0.9f), 0.12f, 0.08f, 0.06f, p.darkMetal);
    b.addBox(Point3(-0.2f, 0.42f, -1.08f), 0.6f, 0.15f, 0.13f, p.hull);
    for (int k = 0; k < 2; ++k) {
      b.addBox(Point3(0.405f, 0.42f, -1.08f + (k == 0 ? -0.065f : 0.065f)), 0.006f, 0.1f, 0.05f, p.rubber);
    }
    b.addBox(Point3(-0.81f, 0.42f, -1.08f), 0.01f, 0.13f, 0.11f, p.hullDark);

    // Smoke grenade launchers on the front corners
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      addSmokeBank(b, p, Point3(0.7f, 0.4f, s * 0.82f), s, 2, 2);
    }

    // Integrated sight unit (gunner, front left) and commander's independent viewer (rear right)
    b.addBox(Point3(0.45f, roof + 0.13f, -0.35f), 0.17f, 0.13f, 0.15f, p.hull);
    b.addBox(Point3(0.625f, roof + 0.14f, -0.35f), 0.006f, 0.08f, 0.11f, p.glass);
    addSightHead(b, p, Point3(-0.45f, roof, 0.45f), 0.3f, 0.12f);

    addHatch(b, p, Point3(-0.35f, roof, -0.38f), 0.24f);
    addHatch(b, p, Point3(0.0f, roof, 0.38f), 0.24f);
    for (int k = 0; k < 4; ++k) {
      float angle = -0.6f + 0.4f * k;
      Point3 dir(cos(angle), 0.0f, sin(angle));
      addVisionBlock(b, p, Point3(0.0f, roof + 0.08f, 0.38f) + dir * 0.28f, dir);
    }
    addAntenna(b, p, Point3(-0.8f, roof, 0.6f), 1.2f);
    b.addRod(Point3(-0.75f, roof, -0.55f), Point3(-0.75f, roof + 0.3f, -0.55f), 0.015f, 6, p.darkMetal);
  }

  void buildGun(TankMeshBuilder &b) const override {
    vehicle_parts::addGun(b, palette, gunSpec());
  }
};

#endif // __BRADLEY_HPP_
