#ifndef __ABRAMS_HPP_
#define __ABRAMS_HPP_

#include <string>
#include "armored_vehicle.hpp"
#include "vehicle_parts.hpp"
#include "../tank/tank_running_gear.hpp"

enum class AbramsVariant { M1A2_SEPV3, M1A2_SEPV4, M1A2D };

/**
 * M1A2 Abrams assembled from replaceable parts:
 *   hull:   running gear, hull body, skirts, optional skirt armor, fittings, engine deck, driver station
 *   turret: body, side fittings, Trophy, optional turret armor, CROWS, loader machine gun, sensors, upgrades
 *   gun:    120 mm M256 (GunPart)
 *   SEPv3:  the base part list
 *   SEPv4:  adds the "sepv4_sensors" part (laser warning receivers, met mast, upgraded sight)
 *   M1A2D:  adds skirt armor, turret armor and the "counter_drone" jammer
 */
class Abrams : public ArmoredVehicle {
public:
  explicit Abrams(AbramsVariant variant = AbramsVariant::M1A2_SEPV3, float turretYaw = 0.0f, float gunPitch = 0.0f)
    : ArmoredVehicle(displayName(variant), turretYaw, gunPitch, defaultPalette(variant)) {
    const TankPalette p = palette;
    setTurretMount(Point3(-0.15f, 1.2f, 0.0f));
    setGunMount(Point3(1.3f, 0.34f, 0.0f));
    setGunPitchLimits(-0.157f, 0.349f);

    std::unique_ptr<ComplexVehiclePart> hull(new ComplexVehiclePart("hull"));
    hull->add(std::unique_ptr<VehiclePart>(new TrackedRunningGearPart("running_gear", p, runningGear())));
    hull->add(makeMeshPart("hull_body", [p](TankMeshBuilder &b) { buildHullBody(b, p); }));
    const float sides[2] = {-1.0f, 1.0f};
    for (int i = 0; i < 2; ++i) {
      const float s = sides[i];
      const char *side = s < 0.0f ? "left" : "right";
      hull->add(makeMeshPart(std::string("skirts_") + side, [p, s](TankMeshBuilder &b) { buildSkirts(b, p, s); }));
      if (variant == AbramsVariant::M1A2D) {
        hull->add(makeMeshPart(std::string("skirt_armor_") + side,
                               [p, s](TankMeshBuilder &b) { buildSkirtArmor(b, p, s); }));
      }
      hull->add(makeMeshPart(std::string("fittings_") + side,
                             [p, s](TankMeshBuilder &b) { buildFittings(b, p, s); }));
    }
    hull->add(makeMeshPart("engine_deck", [p](TankMeshBuilder &b) { buildEngineDeck(b, p); }));
    hull->add(makeMeshPart("driver_station", [p](TankMeshBuilder &b) { buildDriverStation(b, p); }));
    add(std::move(hull));

    std::unique_ptr<ComplexVehiclePart> turret(new ComplexVehiclePart("turret"));
    turret->add(makeMeshPart("turret_body", [p](TankMeshBuilder &b) { buildTurretBody(b, p); }));
    for (int i = 0; i < 2; ++i) {
      const float s = sides[i];
      const char *side = s < 0.0f ? "left" : "right";
      turret->add(makeMeshPart(std::string("turret_side_") + side,
                               [p, s](TankMeshBuilder &b) { buildTurretSide(b, p, s); }));
      std::unique_ptr<VehiclePart> trophy(new TrophyPart(std::string("trophy_") + side, p, s));
      trophy->setLocalTransform(TankTransform::translation(Point3(0.25f, 0.3f, s * 1.14f)));
      turret->add(std::move(trophy));
      if (variant == AbramsVariant::M1A2D) {
        turret->add(makeMeshPart(std::string("turret_armor_") + side,
                                 [p, s](TankMeshBuilder &b) { buildTurretArmor(b, p, s); }));
      }
    }
    turret->add(makeMeshPart("commander_hatch", [p](TankMeshBuilder &b) { buildCommanderHatch(b, p); }));
    std::unique_ptr<ComplexVehiclePart> crows = makeCrows("crows", p);
    crows->setLocalTransform(TankTransform::translation(Point3(-0.1f, turretRoof() + 0.1f, 0.52f)));
    turret->add(std::move(crows));
    turret->add(makeMeshPart("loader_station", [p](TankMeshBuilder &b) { buildLoaderStation(b, p); }));
    std::unique_ptr<VehiclePart> loaderGun(
        new MachineGunPart("loader_mg", p, MachineGunPart::Kind::Medium, 0.5f));
    loaderGun->setLocalTransform(TankTransform::translation(Point3(0.12f, turretRoof() + 0.3f, -0.52f)));
    turret->add(std::move(loaderGun));
    turret->add(makeMeshPart("roof_sensors", [p](TankMeshBuilder &b) { buildRoofSensors(b, p); }));
    if (variant == AbramsVariant::M1A2_SEPV4) {
      turret->add(makeMeshPart("sepv4_sensors", [p](TankMeshBuilder &b) { buildSepv4Sensors(b, p); }));
    }
    if (variant == AbramsVariant::M1A2D) {
      turret->add(makeMeshPart("counter_drone", [p](TankMeshBuilder &b) { buildCounterDrone(b, p); }));
    }
    add(std::move(turret));
    add(std::unique_ptr<VehiclePart>(new GunPart("gun", p, gunSpec())));
  }

  static TankPalette defaultPalette(AbramsVariant variant) {
    if (variant == AbramsVariant::M1A2D) {
      return TankPalette::fromHullColor(0.30f, 0.35f, 0.23f);
    }
    return TankPalette::desertTan();
  }

  static std::string displayName(AbramsVariant variant) {
    switch (variant) {
      case AbramsVariant::M1A2_SEPV4: return "M1A2 SEPv4 Abrams";
      case AbramsVariant::M1A2D: return "M1A2D Abrams";
      default: return "M1A2 SEPv3 Abrams";
    }
  }

  static float turretRoof() { return 0.62f; }

  static TrackedRunningGearSpec runningGear() {
    TrackedRunningGearSpec g;
    g.trackCenterZ = 1.17f;
    g.trackWidth = 0.51f;
    g.trackThickness = 0.065f;
    g.grouserHeight = 0.035f;
    g.shoeSpacing = 0.16f;
    g.roadWheelRadius = 0.26f;
    g.roadWheelGap = 0.035f;
    g.roadWheelTireWidth = 0.18f;
    g.roadWheelX = {-1.86f, -1.24f, -0.62f, 0.0f, 0.62f, 1.24f, 1.86f};
    g.sprocket = {-2.72f, 0.58f, 0.27f};
    g.idler = {2.62f, 0.5f, 0.25f};
    g.sprocketRingOffset = 0.12f;
    g.sprocketHubHalfLength = 0.19f;
    g.sprocketCapOffset = 0.16f;
    g.sprocketTeeth = 11;
    g.returnRollerX = {-1.0f, 0.9f};
    g.returnRollerRadius = 0.08f;
    g.returnRollerDrop = 0.085f;
    g.returnRollerOffset = 0.1f;
    g.returnRollerWidth = 0.1f;
    g.returnRollerAxleEnd = 0.16f;
    g.hullSideZ = 0.88f;
    g.suspensionArmY = 0.48f;
    g.suspensionArmLength = 0.28f;
    g.suspensionArmEndZ = 0.96f;
    return g;
  }

  static vehicle_parts::GunSpec gunSpec() {
    vehicle_parts::GunSpec g;
    g.mantletCenterX = 0.05f;
    g.mantletHalfX = 0.25f;
    g.mantletHalfY = 0.2f;
    g.mantletHalfZ = 0.32f;
    g.coaxZ = 0.22f;
    g.collarStart = 0.3f;
    g.collarLength = 0.25f;
    g.collarRearRadius = 0.14f;
    g.collarFrontRadius = 0.115f;
    g.tubeLength = 3.0f;
    g.tubeRearRadius = 0.105f;
    g.tubeFrontRadius = 0.075f;
    g.evacuatorStart = 0.42f;
    g.evacuatorLength = 0.42f;
    g.evacuatorRadius = 0.115f;
    g.straps = 3;
    g.boreRadius = 0.05f;
    return g;
  }

private:
  static void buildHullBody(TankMeshBuilder &b, const TankPalette &p) {
    Point3 lower[8] = {
      {-2.85f, 0.38f, -0.86f}, {2.62f, 0.38f, -0.86f}, {2.62f, 0.38f, 0.86f}, {-2.85f, 0.38f, 0.86f},
      {-2.95f, 0.95f, -0.86f}, {3.05f, 0.95f, -0.86f}, {3.05f, 0.95f, 0.86f}, {-2.95f, 0.95f, 0.86f}
    };
    b.addHexahedron(lower, p.hullDark);
    Point3 upper[8] = {
      {-2.95f, 0.95f, -1.44f}, {3.05f, 0.95f, -1.44f}, {3.05f, 0.95f, 1.44f}, {-2.95f, 0.95f, 1.44f},
      {-2.95f, 1.2f, -1.44f}, {2.15f, 1.2f, -1.44f}, {2.15f, 1.2f, 1.44f}, {-2.95f, 1.2f, 1.44f}
    };
    b.addHexahedron(upper, p.hull);
  }

  static void buildSkirts(TankMeshBuilder &b, const TankPalette &p, float s) {
    vehicle_parts::addSkirtPanels(b, p, -2.65f, 2.95f, 0.52f, 1.17f, 1.44f, 1.5f, s, 7, 0.28f);
  }

  static void buildSkirtArmor(TankMeshBuilder &b, const TankPalette &p, float s) {
    vehicle_parts::addArmorTiles(b, p, -1.0f, 2.6f, 0.6f, 1.12f, 1.5f, s, 8, 2, 0.06f, p.hull.shade(0.9f));
  }

  static void buildFittings(TankMeshBuilder &b, const TankPalette &p, float s) {
    using namespace vehicle_parts;
    addHeadlight(b, p, Point3(2.7f, 1.13f, s * 1.22f));
    b.addBox(Point3(2.6f, 1.12f, s * 1.0f), 0.05f, 0.04f, 0.06f, p.darkMetal);
    b.addRod(Point3(2.75f, 0.62f, s * 0.6f), Point3(2.95f, 0.62f, s * 0.6f), 0.06f, 10, p.darkMetal);
    b.addBox(Point3(-2.97f, 1.08f, s * 1.25f), 0.03f, 0.05f, 0.08f, p.tailLight);
    b.addRod(Point3(-2.9f, 0.6f, s * 0.55f), Point3(-3.02f, 0.6f, s * 0.55f), 0.06f, 10, p.darkMetal);
    addStowageBox(b, p, Point3(-2.35f, 1.3f, s * 1.22f), 0.4f, 0.1f, 0.16f, p.hullDark);
  }

  static void buildEngineDeck(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    addGrille(b, p, Point3(-2.25f, 1.205f, -0.45f), 0.55f, 0.32f, 9);
    addGrille(b, p, Point3(-2.25f, 1.205f, 0.45f), 0.55f, 0.32f, 9);
    b.addBox(Point3(-2.975f, 0.8f, 0.0f), 0.03f, 0.28f, 0.75f, p.darkMetal);
    for (int k = 0; k < 6; ++k) {
      float y = 0.6f + 0.08f * k;
      b.addBox(Point3(-3.01f, y, 0.0f), 0.015f, 0.015f, 0.72f, p.exhaust);
    }
    b.addBox(Point3(-2.98f, 1.05f, -0.6f), 0.04f, 0.06f, 0.12f, p.darkMetal);
  }

  static void buildDriverStation(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    addHatch(b, p, Point3(1.95f, 1.2f, 0.0f), 0.24f);
    for (int k = -1; k <= 1; ++k) {
      float angle = 0.35f * k;
      addVisionBlock(b, p, Point3(2.2f, 1.24f, 0.0f) + Point3(cos(angle), 0.0f, sin(angle)) * 0.1f,
                     Point3(cos(angle), 0.0f, sin(angle)));
    }
    b.addBox(Point3(2.0f, 1.215f, 0.0f), 0.015f, 0.015f, 1.4f, p.hullDark);
  }

  static void buildTurretBody(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const Point3 Y = axisY();
    const float roof = turretRoof();
    b.addFrustum(Point3(0.0f, 0.0f, 0.0f), Y, 0.08f, 0.92f, 0.92f, 32, p.darkMetal, false, true);
    Point3 body[8] = {
      {-1.2f, 0.06f, -1.18f}, {1.12f, 0.06f, -1.18f}, {1.12f, 0.06f, 1.18f}, {-1.2f, 0.06f, 1.18f},
      {-1.15f, roof, -1.08f}, {0.98f, roof, -1.08f}, {0.98f, roof, 1.08f}, {-1.15f, roof, 1.08f}
    };
    b.addHexahedron(body, p.hull);
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      Point3 cheek[8] = {
        {1.0f, 0.06f, s * 0.36f}, {1.62f, 0.06f, s * 0.36f}, {1.2f, 0.06f, s * 1.18f}, {1.0f, 0.06f, s * 1.18f},
        {0.95f, roof, s * 0.36f}, {1.48f, roof, s * 0.36f}, {1.06f, roof, s * 1.08f}, {0.95f, roof, s * 1.08f}
      };
      b.addHexahedron(cheek, p.hullLight);
    }
    b.addBox(Point3(1.05f, 0.32f, 0.0f), 0.08f, 0.24f, 0.36f, p.hullDark);
    Point3 bustle[8] = {
      {-2.0f, 0.14f, -1.0f}, {-1.15f, 0.14f, -1.0f}, {-1.15f, 0.14f, 1.0f}, {-2.0f, 0.14f, 1.0f},
      {-2.08f, 0.6f, -0.98f}, {-1.15f, 0.6f, -0.98f}, {-1.15f, 0.6f, 0.98f}, {-2.08f, 0.6f, 0.98f}
    };
    b.addHexahedron(bustle, p.hull);
    addBasket(b, p, -2.04f, -2.55f, 0.95f, 0.22f, 0.62f);
    addTarpRoll(b, p, Point3(-2.32f, 0.38f, 0.0f), 0.13f, 1.6f);
    b.addBox(Point3(-2.3f, 0.58f, 0.45f), 0.14f, 0.07f, 0.28f, p.hullDark);
  }

  static void buildTurretSide(TankMeshBuilder &b, const TankPalette &p, float s) {
    using namespace vehicle_parts;
    addStowageBox(b, p, Point3(-1.35f, 0.34f, s * 1.27f), 0.55f, 0.2f, 0.1f, p.hull.shade(0.95f));
    addSmokeBank(b, p, Point3(-0.45f, 0.5f, s * 1.18f), s, 3, 2);
  }

  static void buildTurretArmor(TankMeshBuilder &b, const TankPalette &p, float s) {
    vehicle_parts::addArmorTiles(b, p, -1.95f, -0.85f, 0.1f, 0.55f, 1.37f, s, 4, 2, 0.05f, p.hull.shade(0.9f));
  }

  static void buildCommanderHatch(TankMeshBuilder &b, const TankPalette &p) {
    vehicle_parts::addHatch(b, p, Point3(-0.35f, turretRoof(), 0.52f), 0.3f);
  }

  static void buildLoaderStation(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const Point3 Y = axisY();
    const float roof = turretRoof();
    addHatch(b, p, Point3(-0.35f, roof, -0.52f), 0.28f);
    b.addFrustum(Point3(-0.35f, roof, -0.52f), Y, 0.06f, 0.33f, 0.33f, 24, p.darkMetal, false, true);
    b.addBox(Point3(0.02f, roof + 0.25f, -0.52f), 0.02f, 0.18f, 0.25f, p.hull);
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      b.addOrientedBox(Point3(-0.12f, roof + 0.25f, -0.52f + s * 0.3f), Point3(0.6f, 0.0f, s * 0.8f),
                       Y, Point3(-s * 0.8f, 0.0f, 0.6f), 0.02f, 0.18f, 0.12f, p.hull);
    }
  }

  static void buildRoofSensors(TankMeshBuilder &b, const TankPalette &p) {
    using namespace vehicle_parts;
    const Point3 Y = axisY();
    const float roof = turretRoof();
    b.addBox(Point3(0.7f, roof + 0.13f, 0.62f), 0.2f, 0.13f, 0.16f, p.hull);
    b.addBox(Point3(0.905f, roof + 0.14f, 0.62f), 0.006f, 0.08f, 0.12f, p.glass);
    b.addBox(Point3(0.75f, roof + 0.27f, 0.62f), 0.13f, 0.015f, 0.17f, p.hullDark);
    addSightHead(b, p, Point3(0.55f, roof, -0.62f), 0.24f, 0.12f);
    b.addRod(Point3(-0.95f, roof, 0.0f), Point3(-0.95f, roof + 0.38f, 0.0f), 0.018f, 8, p.darkMetal);
    b.addFrustum(Point3(-0.95f, roof + 0.38f, 0.0f), Y, 0.04f, 0.04f, 0.03f, 10, p.darkMetal, true, true);
    addAntenna(b, p, Point3(-1.0f, roof, 0.92f), 1.3f);
    addAntenna(b, p, Point3(-1.0f, roof, -0.92f), 1.1f);
    b.addBox(Point3(-0.75f, roof + 0.06f, -0.3f), 0.12f, 0.06f, 0.1f, p.hullDark);
  }

  static void buildSepv4Sensors(TankMeshBuilder &b, const TankPalette &p) {
    const float roof = turretRoof();
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      b.addBox(Point3(0.9f, roof + 0.04f, s * 0.95f), 0.06f, 0.04f, 0.06f, p.darkMetal);
      b.addBox(Point3(-1.1f, roof + 0.04f, s * 0.95f), 0.06f, 0.04f, 0.06f, p.darkMetal);
    }
    b.addRod(Point3(-0.7f, roof, 0.3f), Point3(-0.7f, roof + 0.55f, 0.3f), 0.015f, 6, p.darkMetal);
    b.addBox(Point3(-0.7f, roof + 0.58f, 0.3f), 0.05f, 0.03f, 0.05f, p.hullLight);
    b.addBox(Point3(0.7f, roof + 0.31f, 0.62f), 0.1f, 0.03f, 0.1f, p.darkMetal);
  }

  static void buildCounterDrone(TankMeshBuilder &b, const TankPalette &p) {
    b.addBox(Point3(-1.65f, 0.68f, 0.0f), 0.22f, 0.08f, 0.3f, p.darkMetal);
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      b.addRod(Point3(-1.55f, 0.76f, s * 0.2f), Point3(-1.55f, 1.0f, s * 0.2f), 0.012f, 6, p.darkMetal);
    }
  }
};

#endif // __ABRAMS_HPP_
