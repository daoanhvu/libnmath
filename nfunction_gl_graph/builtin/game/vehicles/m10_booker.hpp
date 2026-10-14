#ifndef __M10_BOOKER_HPP_
#define __M10_BOOKER_HPP_

#include <string>
#include "armored_vehicle.hpp"
#include "vehicle_parts.hpp"
#include "../tank/tank_running_gear.hpp"

enum class BookerVariant { M10_BOOKER, MPF_PROTOTYPE };

/**
 * M10 Booker / Mobile Protected Firepower, sharing a hull and a 105 mm gun.
 *   M10_BOOKER:    faceted turret, CROWS ("crows") and a loader M240 ("loader_mg")
 *   MPF_PROTOTYPE: flatter turret, pintle M2 ("commander_mg") and loader M240 ("loader_mg")
 * The muzzle brake is part of the "gun" spec, so replacing "gun" changes the cannon.
 */
class M10Booker : public ArmoredVehicle {
public:
  explicit M10Booker(BookerVariant variant = BookerVariant::M10_BOOKER, float turretYaw = 0.0f,
                     float gunPitch = 0.0f)
    : ArmoredVehicle(variant == BookerVariant::MPF_PROTOTYPE ? "Mobile Protected Firepower" : "M10 Booker",
                     turretYaw, gunPitch, TankPalette::desertTan()) {
    const TankPalette p = palette;
    setTurretMount(Point3(-0.45f, 1.5f, 0.0f));
    setGunMount(variant == BookerVariant::MPF_PROTOTYPE ? Point3(1.15f, 0.3f, 0.0f)
                                                        : Point3(1.3f, 0.36f, 0.0f));
    setGunPitchLimits(-0.17f, 0.35f);

    std::unique_ptr<ComplexVehiclePart> hull(new ComplexVehiclePart("hull"));
    hull->add(std::unique_ptr<VehiclePart>(new TrackedRunningGearPart("running_gear", p, runningGear())));
    hull->add(makeMeshPart("hull_body", [p](TankMeshBuilder &b) { buildHullBody(b, p); }));
    add(std::move(hull));

    std::unique_ptr<ComplexVehiclePart> turret(new ComplexVehiclePart("turret"));
    if (variant == BookerVariant::MPF_PROTOTYPE) {
      assembleMpfTurret(*turret, p);
    } else {
      assembleBookerTurret(*turret, p);
    }
    add(std::move(turret));
    add(std::unique_ptr<VehiclePart>(new GunPart("gun", p, gunSpec(variant))));
  }

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

  static vehicle_parts::GunSpec gunSpec(BookerVariant variant) {
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

private:
  static float bookerRoof() { return 0.58f; }
  static float mpfRoof() { return 0.5f; }

  static void assembleBookerTurret(ComplexVehiclePart &turret, const TankPalette &p) {
    turret.add(makeMeshPart("turret_body", [p](TankMeshBuilder &b) { buildBookerBody(b, p); }));
    std::unique_ptr<ComplexVehiclePart> crows = makeCrows("crows", p);
    crows->setLocalTransform(TankTransform::translation(Point3(-0.95f, bookerRoof() + 0.04f, 0.45f)));
    turret.add(std::move(crows));
    turret.add(makeMeshPart("loader_hatch", [p](TankMeshBuilder &b) { buildBookerLoaderHatch(b, p); }));
    std::unique_ptr<VehiclePart> loaderGun(
        new MachineGunPart("loader_mg", p, MachineGunPart::Kind::Medium, 0.45f));
    loaderGun->setLocalTransform(TankTransform::translation(Point3(-0.15f, bookerRoof() + 0.3f, -0.45f)));
    turret.add(std::move(loaderGun));
    turret.add(makeMeshPart("roof_sensors", [p](TankMeshBuilder &b) { buildBookerSensors(b, p); }));
  }

  static void assembleMpfTurret(ComplexVehiclePart &turret, const TankPalette &p) {
    turret.add(makeMeshPart("turret_body", [p](TankMeshBuilder &b) { buildMpfBody(b, p); }));
    std::unique_ptr<VehiclePart> commanderGun(
        new MachineGunPart("commander_mg", p, MachineGunPart::Kind::Heavy, 0.6f));
    commanderGun->setLocalTransform(TankTransform::translation(Point3(-0.05f, mpfRoof() + 0.36f, 0.5f)));
    turret.add(std::move(commanderGun));
    turret.add(makeMeshPart("loader_station", [p](TankMeshBuilder &b) { buildMpfLoaderStation(b, p); }));
    std::unique_ptr<ComplexVehiclePart> loader(new ComplexVehiclePart("loader_mg"));
    loader->setLocalTransform(TankTransform::translation(Point3(-0.1f, mpfRoof() + 0.2f, -0.5f)));
    loader->add(makeMeshPart("loader_mg.pintle", [p](TankMeshBuilder &b) {
      b.addRod(Point3(0.0f, -0.2f, 0.0f), Point3(0.0f, 0.0f, 0.0f), 0.02f, 8, p.darkMetal);
    }));
    loader->add(std::unique_ptr<VehiclePart>(
        new MachineGunPart("loader_mg.gun", p, MachineGunPart::Kind::Medium, 0.45f)));
    turret.add(std::move(loader));
    turret.add(makeMeshPart("roof_sensors", [p](TankMeshBuilder &b) { buildMpfSensors(b, p); }));
  }

  static void buildHullBody(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    Point3 lower[8] = {
      {-2.7f, 0.42f, -0.84f}, {2.55f, 0.42f, -0.84f}, {2.55f, 0.42f, 0.84f}, {-2.7f, 0.42f, 0.84f},
      {-2.8f, 1.0f, -0.84f}, {2.95f, 1.0f, -0.84f}, {2.95f, 1.0f, 0.84f}, {-2.8f, 1.0f, 0.84f}
    };
    b.addHexahedron(lower, p.hullDark);
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

    addGrille(b, p, Point3(1.55f, 1.505f, 0.6f), 0.5f, 0.42f, 8);
    addGrille(b, p, Point3(0.75f, 1.505f, 0.6f), 0.22f, 0.42f, 4);
    b.addBox(Point3(1.2f, 1.25f, 1.405f), 0.35f, 0.14f, 0.01f, p.darkMetal);
    for (int k = 0; k < 5; ++k) {
      b.addBox(Point3(1.2f, 1.15f + 0.05f * k, 1.42f), 0.33f, 0.012f, 0.01f, p.exhaust);
    }
    addHatch(b, p, Point3(1.75f, 1.5f, -0.6f), 0.24f);
    for (int k = -1; k <= 1; ++k) {
      float angle = 0.4f * k;
      Point3 dir(cos(angle), 0.0f, sin(angle));
      addVisionBlock(b, p, Point3(2.0f, 1.54f, -0.6f) + dir * 0.08f, dir);
    }
    b.addBox(Point3(-2.815f, 1.05f, 0.0f), 0.015f, 0.32f, 0.5f, p.hull.shade(0.92f));
    b.addBox(Point3(-2.835f, 1.05f, 0.0f), 0.01f, 0.02f, 0.15f, p.darkMetal);
    for (int k = -1; k <= 1; k += 2) {
      b.addBox(Point3(-2.83f, 1.05f + 0.2f * k, -0.5f), 0.02f, 0.05f, 0.03f, p.darkMetal);
    }
    b.addRod(Point3(2.5f, 1.4f, -1.1f), Point3(2.5f, 1.4f, 1.1f), 0.025f, 8, p.steel.shade(0.7f));
  }

  static void buildBookerBody(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const Point3 Y = axisY();
    const float roof = bookerRoof();
    b.addFrustum(Point3(0.0f, 0.0f, 0.0f), Y, 0.06f, 0.85f, 0.85f, 32, p.darkMetal, false, true);
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
    addHatch(b, p, Point3(-0.45f, roof, 0.45f), 0.27f);
    addSightHead(b, p, Point3(0.3f, roof, 0.55f), 0.28f, 0.11f);
  }

  static void buildBookerLoaderHatch(TankMeshBuilder &b, const TankPalette &p) {
    const float roof = bookerRoof();
    vehicle_parts::addHatch(b, p, Point3(-0.45f, roof, -0.45f), 0.25f);
    b.addRod(Point3(-0.2f, roof, -0.45f), Point3(-0.2f, roof + 0.25f, -0.45f), 0.02f, 8, p.darkMetal);
  }

  static void buildBookerSensors(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const float roof = bookerRoof();
    b.addBox(Point3(0.65f, roof + 0.12f, -0.55f), 0.16f, 0.12f, 0.14f, p.hull);
    b.addBox(Point3(0.815f, roof + 0.13f, -0.55f), 0.006f, 0.07f, 0.1f, p.glass);
    addAntenna(b, p, Point3(-1.1f, roof, 0.8f), 1.2f);
    addAntenna(b, p, Point3(-1.1f, roof, -0.8f), 1.0f);
    b.addRod(Point3(-0.95f, roof, -0.1f), Point3(-0.95f, roof + 0.3f, -0.1f), 0.016f, 8, p.darkMetal);
  }

  static void buildMpfBody(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const Point3 Y = axisY();
    const float roof = mpfRoof();
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
    addHatch(b, p, Point3(-0.4f, roof, 0.5f), 0.3f);
    b.addFrustum(Point3(-0.4f, roof, 0.5f), Y, 0.08f, 0.36f, 0.36f, 24, p.hullDark, false, true);
    b.addRod(Point3(-0.1f, roof + 0.08f, 0.5f), Point3(-0.1f, roof + 0.3f, 0.5f), 0.025f, 8, p.darkMetal);
  }

  static void buildMpfLoaderStation(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const float roof = mpfRoof();
    addSightHead(b, p, Point3(0.35f, roof, 0.75f), 0.22f, 0.1f);
    addHatch(b, p, Point3(-0.4f, roof, -0.5f), 0.26f);
  }

  static void buildMpfSensors(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const float roof = mpfRoof();
    b.addBox(Point3(0.6f, roof + 0.12f, -0.6f), 0.18f, 0.12f, 0.15f, p.hull);
    b.addBox(Point3(0.785f, roof + 0.13f, -0.6f), 0.006f, 0.07f, 0.11f, p.glass);
    addAntenna(b, p, Point3(-1.05f, roof, 0.85f), 1.2f);
    addAntenna(b, p, Point3(-1.05f, roof, -0.85f), 1.0f);
  }
};

#endif // __M10_BOOKER_HPP_
