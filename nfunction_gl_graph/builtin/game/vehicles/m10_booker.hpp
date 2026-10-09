#ifndef __M10_BOOKER_HPP_
#define __M10_BOOKER_HPP_

#include "armored_vehicle.hpp"
#include "vehicle_parts.hpp"
#include "../tank/tank_running_gear.hpp"

enum class BookerVariant { M10_BOOKER, MPF_PROTOTYPE };

/**
 * M10 Booker / Mobile Protected Firepower: tall hull with the engine and drive sprocket at the front,
 * six road wheels, turret set back on the hull, 105 mm gun with bore evacuator and muzzle brake.
 *   M10_BOOKER:    faceted turret with CROWS and a commander's panoramic sight
 *   MPF_PROTOTYPE: flatter Abrams-style turret with a long bustle and a pintle M2
 */
class M10Booker : public ArmoredVehicle {
public:
  explicit M10Booker(BookerVariant variant = BookerVariant::M10_BOOKER, float turretYaw = 0.0f, float gunPitch = 0.0f)
    : ArmoredVehicle(turretYaw, gunPitch, TankPalette::desertTan()), variant(variant) {}

  std::string name() const override {
    return variant == BookerVariant::MPF_PROTOTYPE ? "Mobile Protected Firepower" : "M10 Booker";
  }

  Point3 turretMount() const override { return Point3(-0.45f, 1.5f, 0.0f); }
  Point3 gunMount() const override {
    return variant == BookerVariant::MPF_PROTOTYPE ? Point3(1.15f, 0.3f, 0.0f) : Point3(1.3f, 0.36f, 0.0f);
  }
  float minGunPitch() const override { return -0.17f; }
  float maxGunPitch() const override { return 0.35f; }

  static TrackedRunningGearSpec runningGear() {
    TrackedRunningGearSpec g;
    g.trackCenterZ = 1.12f;
    g.trackWidth = 0.48f;
    g.trackThickness = 0.065f;
    g.grouserHeight = 0.035f;
    g.shoeSpacing = 0.17f;
    g.roadWheelRadius = 0.29f;
    g.roadWheelGap = 0.035f;
    g.roadWheelTireWidth = 0.17f;
    g.roadWheelX = {-1.8f, -1.08f, -0.36f, 0.36f, 1.08f, 1.8f};
    g.sprocket = {2.5f, 0.66f, 0.27f};
    g.idler = {-2.55f, 0.55f, 0.26f};
    g.sprocketRingOffset = 0.11f;
    g.sprocketHubHalfLength = 0.18f;
    g.sprocketCapOffset = 0.15f;
    g.sprocketTeeth = 10;
    g.returnRollerX = {-1.45f, 0.0f, 1.45f};
    g.returnRollerRadius = 0.08f;
    g.returnRollerDrop = 0.085f;
    g.returnRollerOffset = 0.1f;
    g.returnRollerWidth = 0.1f;
    g.returnRollerAxleEnd = 0.16f;
    g.hullSideZ = 0.84f;
    g.suspensionArmY = 0.52f;
    g.suspensionArmLength = 0.3f;
    g.suspensionArmEndZ = 0.92f;
    return g;
  }

  vehicle_parts::GunSpec gunSpec() const {
    vehicle_parts::GunSpec g;
    g.mantletCenterX = 0.02f;
    g.mantletHalfX = 0.2f;
    g.mantletHalfY = 0.18f;
    g.mantletHalfZ = 0.28f;
    g.coaxZ = 0.2f;
    g.collarStart = 0.2f;
    g.collarLength = 0.22f;
    g.collarRearRadius = 0.13f;
    g.collarFrontRadius = 0.105f;
    g.tubeLength = 2.6f;
    g.tubeRearRadius = 0.095f;
    g.tubeFrontRadius = 0.07f;
    g.evacuatorStart = 0.4f;
    g.evacuatorLength = 0.38f;
    g.evacuatorRadius = 0.11f;
    g.straps = 2;
    g.muzzleSensor = false;
    if (variant == BookerVariant::MPF_PROTOTYPE) {
      g.muzzleBrakeLength = 0.22f;
      g.muzzleBrakeRadius = 0.09f;
      g.muzzleBrakePorts = 2;
    } else {
      g.muzzleBrakeLength = 0.32f;
      g.muzzleBrakeRadius = 0.1f;
      g.muzzleBrakePorts = 3;
    }
    g.boreRadius = 0.045f;
    return g;
  }

protected:
  void buildHull(TankMeshBuilder &b) const override {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    TrackedRunningGear gear(runningGear(), p);
    gear.build(b, 1.0f);
    gear.build(b, -1.0f);

    Point3 lower[8] = {
      {-2.7f, 0.42f, -0.84f}, {2.55f, 0.42f, -0.84f}, {2.55f, 0.42f, 0.84f}, {-2.7f, 0.42f, 0.84f},
      {-2.8f, 1.0f, -0.84f}, {2.95f, 1.0f, -0.84f}, {2.95f, 1.0f, 0.84f}, {-2.8f, 1.0f, 0.84f}
    };
    b.addHexahedron(lower, p.hullDark);

    // Tall upper hull with a short, steep upper glacis
    Point3 upper[8] = {
      {-2.8f, 1.0f, -1.4f}, {2.95f, 1.0f, -1.4f}, {2.95f, 1.0f, 1.4f}, {-2.8f, 1.0f, 1.4f},
      {-2.8f, 1.5f, -1.36f}, {2.3f, 1.5f, -1.36f}, {2.3f, 1.5f, 1.36f}, {-2.8f, 1.5f, 1.36f}
    };
    b.addHexahedron(upper, p.hull);

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      addSkirtPanels(b, p, -2.55f, 2.85f, 0.52f, 1.0f, 1.4f, 1.46f, s, 6, 0.3f);
      addHeadlight(b, p, Point3(2.62f, 1.3f, s * 1.18f));
      b.addRod(Point3(2.7f, 0.7f, s * 0.55f), Point3(2.9f, 0.7f, s * 0.55f), 0.06f, 10, p.darkMetal);
      b.addBox(Point3(-2.82f, 1.35f, s * 1.2f), 0.03f, 0.05f, 0.08f, p.tailLight);
      b.addRod(Point3(-2.75f, 0.65f, s * 0.55f), Point3(-2.9f, 0.65f, s * 0.55f), 0.06f, 10, p.darkMetal);
      addStowageBox(b, p, Point3(-2.1f, 1.6f, s * 1.18f), 0.5f, 0.1f, 0.16f, p.hullDark);
    }

    // Front engine compartment: grilles on the right, exhaust on the right side
    addGrille(b, p, Point3(1.55f, 1.505f, 0.6f), 0.5f, 0.42f, 8);
    addGrille(b, p, Point3(0.75f, 1.505f, 0.6f), 0.22f, 0.42f, 4);
    b.addBox(Point3(1.2f, 1.25f, 1.405f), 0.35f, 0.14f, 0.01f, p.darkMetal);
    for (int k = 0; k < 5; ++k) {
      b.addBox(Point3(1.2f, 1.15f + 0.05f * k, 1.42f), 0.33f, 0.012f, 0.01f, p.exhaust);
    }

    // Driver hatch with periscopes, front left
    addHatch(b, p, Point3(1.75f, 1.5f, -0.6f), 0.24f);
    for (int k = -1; k <= 1; ++k) {
      float angle = 0.4f * k;
      Point3 dir(cos(angle), 0.0f, sin(angle));
      addVisionBlock(b, p, Point3(2.0f, 1.54f, -0.6f) + dir * 0.08f, dir);
    }

    // Rear ammunition door with hinges and handle
    b.addBox(Point3(-2.815f, 1.05f, 0.0f), 0.015f, 0.32f, 0.5f, p.hull.shade(0.92f));
    b.addBox(Point3(-2.835f, 1.05f, 0.0f), 0.01f, 0.02f, 0.15f, p.darkMetal);
    for (int k = -1; k <= 1; k += 2) {
      b.addBox(Point3(-2.83f, 1.05f + 0.2f * k, -0.5f), 0.02f, 0.05f, 0.03f, p.darkMetal);
    }

    // Towing cable along the upper glacis
    b.addRod(Point3(2.5f, 1.4f, -1.1f), Point3(2.5f, 1.4f, 1.1f), 0.025f, 8, p.steel.shade(0.7f));
  }

  void buildTurret(TankMeshBuilder &b) const override {
    if (variant == BookerVariant::MPF_PROTOTYPE) {
      buildMpfTurret(b);
    } else {
      buildBookerTurret(b);
    }
  }

  void buildGun(TankMeshBuilder &b) const override {
    vehicle_parts::addGun(b, palette, gunSpec());
  }

private:
  BookerVariant variant;

  void buildBookerTurret(TankMeshBuilder &b) const {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    const Point3 Y = axisY();
    const float roof = 0.58f;

    b.addFrustum(Point3(0.0f, 0.0f, 0.0f), Y, 0.06f, 0.85f, 0.85f, 32, p.darkMetal, false, true);

    // Faceted body: sides lean in, lower side plates flare out
    Point3 body[8] = {
      {-1.3f, 0.05f, -1.05f}, {1.0f, 0.05f, -1.05f}, {1.0f, 0.05f, 1.05f}, {-1.3f, 0.05f, 1.05f},
      {-1.22f, roof, -0.9f}, {0.86f, roof, -0.9f}, {0.86f, roof, 0.9f}, {-1.22f, roof, 0.9f}
    };
    b.addHexahedron(body, p.hull);
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      Point3 cheek[8] = {
        {0.95f, 0.05f, s * 0.32f}, {1.48f, 0.05f, s * 0.32f}, {1.05f, 0.05f, s * 1.05f}, {0.95f, 0.05f, s * 1.05f},
        {0.85f, roof, s * 0.32f}, {1.28f, roof, s * 0.32f}, {0.9f, roof, s * 0.9f}, {0.85f, roof, s * 0.9f}
      };
      b.addHexahedron(cheek, p.hullLight);
      Point3 sidePlate[8] = {
        {-1.1f, 0.08f, s * 1.05f}, {0.9f, 0.08f, s * 1.05f}, {0.9f, 0.08f, s * 1.13f}, {-1.1f, 0.08f, s * 1.13f},
        {-1.05f, 0.34f, s * 1.0f}, {0.85f, 0.34f, s * 1.0f}, {0.85f, 0.34f, s * 1.06f}, {-1.05f, 0.34f, s * 1.06f}
      };
      b.addHexahedron(sidePlate, p.hull.shade(0.94f));
      addSmokeBank(b, p, Point3(0.35f, 0.45f, s * 0.98f), s, 3, 2);
      addStowageBox(b, p, Point3(-1.0f, 0.45f, s * 1.06f), 0.3f, 0.1f, 0.1f, p.hullDark);
    }
    b.addBox(Point3(0.98f, 0.3f, 0.0f), 0.06f, 0.22f, 0.32f, p.hullDark);

    Point3 bustle[8] = {
      {-1.85f, 0.12f, -0.85f}, {-1.25f, 0.12f, -0.85f}, {-1.25f, 0.12f, 0.85f}, {-1.85f, 0.12f, 0.85f},
      {-1.92f, 0.55f, -0.82f}, {-1.2f, 0.55f, -0.82f}, {-1.2f, 0.55f, 0.82f}, {-1.92f, 0.55f, 0.82f}
    };
    b.addHexahedron(bustle, p.hull);
    addBasket(b, p, -1.9f, -2.3f, 0.8f, 0.2f, 0.55f);
    addTarpRoll(b, p, Point3(-2.1f, 0.33f, 0.0f), 0.12f, 1.4f);

    // Commander (right): hatch, panoramic sight, CROWS
    addHatch(b, p, Point3(-0.45f, roof, 0.45f), 0.27f);
    addSightHead(b, p, Point3(0.3f, roof, 0.55f), 0.28f, 0.11f);
    {
      TransformScope crows(b, TankTransform::translation(Point3(-0.95f, roof + 0.04f, 0.45f)));
      addCrows(b, p);
    }

    // Loader (left): hatch and M240
    addHatch(b, p, Point3(-0.45f, roof, -0.45f), 0.25f);
    b.addRod(Point3(-0.2f, roof, -0.45f), Point3(-0.2f, roof + 0.25f, -0.45f), 0.02f, 8, p.darkMetal);
    {
      TransformScope mg(b, TankTransform::translation(Point3(-0.15f, roof + 0.3f, -0.45f)));
      addMachineGun(b, p, 0.45f);
    }

    // Gunner's sight, antennas, wind sensor
    b.addBox(Point3(0.65f, roof + 0.12f, -0.55f), 0.16f, 0.12f, 0.14f, p.hull);
    b.addBox(Point3(0.815f, roof + 0.13f, -0.55f), 0.006f, 0.07f, 0.1f, p.glass);
    addAntenna(b, p, Point3(-1.1f, roof, 0.8f), 1.2f);
    addAntenna(b, p, Point3(-1.1f, roof, -0.8f), 1.0f);
    b.addRod(Point3(-0.95f, roof, -0.1f), Point3(-0.95f, roof + 0.3f, -0.1f), 0.016f, 8, p.darkMetal);
  }

  void buildMpfTurret(TankMeshBuilder &b) const {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    const Point3 Y = axisY();
    const float roof = 0.5f;

    b.addFrustum(Point3(0.0f, 0.0f, 0.0f), Y, 0.06f, 0.85f, 0.85f, 32, p.darkMetal, false, true);

    Point3 body[8] = {
      {-1.25f, 0.05f, -1.12f}, {0.95f, 0.05f, -1.12f}, {0.95f, 0.05f, 1.12f}, {-1.25f, 0.05f, 1.12f},
      {-1.2f, roof, -1.02f}, {0.82f, roof, -1.02f}, {0.82f, roof, 1.02f}, {-1.2f, roof, 1.02f}
    };
    b.addHexahedron(body, p.hull);
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      Point3 cheek[8] = {
        {0.9f, 0.05f, s * 0.3f}, {1.38f, 0.05f, s * 0.3f}, {1.05f, 0.05f, s * 1.12f}, {0.9f, 0.05f, s * 1.12f},
        {0.8f, roof, s * 0.3f}, {1.24f, roof, s * 0.3f}, {0.92f, roof, s * 1.02f}, {0.8f, roof, s * 1.02f}
      };
      b.addHexahedron(cheek, p.hullLight);
      addSmokeBank(b, p, Point3(0.2f, 0.38f, s * 1.13f), s, 4, 1);
      addStowageBox(b, p, Point3(-1.25f, 0.3f, s * 1.18f), 0.45f, 0.16f, 0.08f, p.hull.shade(0.95f));
    }
    b.addBox(Point3(0.92f, 0.26f, 0.0f), 0.06f, 0.2f, 0.3f, p.hullDark);

    Point3 bustle[8] = {
      {-2.05f, 0.1f, -0.95f}, {-1.2f, 0.1f, -0.95f}, {-1.2f, 0.1f, 0.95f}, {-2.05f, 0.1f, 0.95f},
      {-2.12f, 0.48f, -0.92f}, {-1.2f, 0.48f, -0.92f}, {-1.2f, 0.48f, 0.92f}, {-2.12f, 0.48f, 0.92f}
    };
    b.addHexahedron(bustle, p.hull);
    addBasket(b, p, -2.1f, -2.5f, 0.9f, 0.18f, 0.5f);
    b.addBox(Point3(-2.3f, 0.32f, -0.4f), 0.15f, 0.12f, 0.3f, p.canvas);

    // Commander (right): cupola ring, pintle M2, panoramic sight
    addHatch(b, p, Point3(-0.4f, roof, 0.5f), 0.3f);
    b.addFrustum(Point3(-0.4f, roof, 0.5f), Y, 0.08f, 0.36f, 0.36f, 24, p.hullDark, false, true);
    b.addRod(Point3(-0.1f, roof + 0.08f, 0.5f), Point3(-0.1f, roof + 0.3f, 0.5f), 0.025f, 8, p.darkMetal);
    {
      TransformScope mg(b, TankTransform::translation(Point3(-0.05f, roof + 0.36f, 0.5f)));
      addHeavyMachineGun(b, p, 0.6f);
    }
    addSightHead(b, p, Point3(0.35f, roof, 0.75f), 0.22f, 0.1f);

    // Loader (left)
    addHatch(b, p, Point3(-0.4f, roof, -0.5f), 0.26f);
    {
      TransformScope mg(b, TankTransform::translation(Point3(-0.1f, roof + 0.2f, -0.5f)));
      b.addRod(Point3(0.0f, -0.2f, 0.0f), Point3(0.0f, 0.0f, 0.0f), 0.02f, 8, p.darkMetal);
      addMachineGun(b, p, 0.45f);
    }

    // Gunner's sight doghouse, antennas
    b.addBox(Point3(0.6f, roof + 0.12f, -0.6f), 0.18f, 0.12f, 0.15f, p.hull);
    b.addBox(Point3(0.785f, roof + 0.13f, -0.6f), 0.006f, 0.07f, 0.11f, p.glass);
    addAntenna(b, p, Point3(-1.05f, roof, 0.85f), 1.2f);
    addAntenna(b, p, Point3(-1.05f, roof, -0.85f), 1.0f);
  }
};

#endif // __M10_BOOKER_HPP_
