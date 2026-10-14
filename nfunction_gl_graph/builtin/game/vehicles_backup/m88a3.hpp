#ifndef __M88A3_HPP_
#define __M88A3_HPP_

#include "armored_vehicle.hpp"
#include "vehicle_parts.hpp"
#include "../tank/tank_running_gear.hpp"

/**
 * M88A3 HERCULES armored recovery vehicle: long hull with a tall crew cab at the front,
 * dozer spade on the bow, rear engine deck and an A-frame lifting boom.
 * There is no main gun, so the parts are reused:
 *   turret = commander's cupola with the .50 cal (turret yaw)
 *   gun    = A-frame boom, stowed lying back over the deck; gun pitch raises it over the bow
 */
class M88A3 : public ArmoredVehicle {
public:
  explicit M88A3(float cupolaYaw = 0.0f, float boomRaise = 0.0f)
    : ArmoredVehicle(cupolaYaw, boomRaise, TankPalette::desertTan()) {}

  std::string name() const override { return "M88A3 Recovery Vehicle"; }

  Point3 turretMount() const override { return Point3(1.25f, cabRoof(), 0.0f); }
  Point3 gunMount() const override { return Point3(0.0f, 0.0f, 0.0f); }
  float minGunPitch() const override { return 0.0f; }
  float maxGunPitch() const override { return 2.0f; }

  static float cabRoof() { return 2.4f; }
  static Point3 boomPivot() { return Point3(2.35f, 2.52f, 0.0f); }
  static float boomLength() { return 4.2f; }

  /** The boom is not carried by the cupola: it pivots on the hull, rotating its tip up over the bow. */
  TankTransform gunTransform() const override {
    return TankTransform::translation(boomPivot()) * TankTransform::rotationZ(-gunPitch());
  }

  static TrackedRunningGearSpec runningGear() {
    TrackedRunningGearSpec g;
    g.trackCenterZ = 1.3f;
    g.trackWidth = 0.55f;
    g.trackThickness = 0.07f;
    g.grouserHeight = 0.04f;
    g.shoeSpacing = 0.18f;
    g.roadWheelRadius = 0.3f;
    g.roadWheelGap = 0.04f;
    g.roadWheelTireWidth = 0.19f;
    g.roadWheelX = {-2.1f, -1.26f, -0.42f, 0.42f, 1.26f, 2.1f};
    g.sprocket = {-2.9f, 0.68f, 0.3f};
    g.idler = {2.85f, 0.6f, 0.28f};
    g.sprocketRingOffset = 0.13f;
    g.sprocketHubHalfLength = 0.2f;
    g.sprocketCapOffset = 0.17f;
    g.sprocketTeeth = 11;
    g.returnRollerX = {-1.4f, 0.0f, 1.4f};
    g.returnRollerRadius = 0.09f;
    g.returnRollerDrop = 0.095f;
    g.returnRollerOffset = 0.12f;
    g.returnRollerWidth = 0.1f;
    g.returnRollerAxleEnd = 0.17f;
    g.hullSideZ = 1.02f;
    g.suspensionArmY = 0.55f;
    g.suspensionArmLength = 0.3f;
    g.suspensionArmEndZ = 1.1f;
    return g;
  }

protected:
  void buildHull(TankMeshBuilder &b) const override {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    const float deck = 1.38f;
    const float roof = cabRoof();
    TrackedRunningGear gear(runningGear(), p);
    gear.build(b, 1.0f);
    gear.build(b, -1.0f);

    Point3 lower[8] = {
      {-3.0f, 0.45f, -1.02f}, {2.75f, 0.45f, -1.02f}, {2.75f, 0.45f, 1.02f}, {-3.0f, 0.45f, 1.02f},
      {-3.05f, deck, -1.02f}, {3.15f, deck, -1.02f}, {3.15f, deck, 1.02f}, {-3.05f, deck, 1.02f}
    };
    b.addHexahedron(lower, p.hullDark);
    Point3 fenders[8] = {
      {-3.05f, 1.13f, -1.62f}, {3.1f, 1.13f, -1.62f}, {3.1f, 1.13f, 1.62f}, {-3.05f, 1.13f, 1.62f},
      {-3.05f, deck, -1.62f}, {3.15f, deck, -1.62f}, {3.15f, deck, 1.62f}, {-3.05f, deck, 1.62f}
    };
    b.addHexahedron(fenders, p.hull);

    // Crew cab with a raked front
    Point3 cab[8] = {
      {0.1f, deck, -1.38f}, {3.1f, deck, -1.38f}, {3.1f, deck, 1.38f}, {0.1f, deck, 1.38f},
      {0.15f, roof, -1.28f}, {2.5f, roof, -1.28f}, {2.5f, roof, 1.28f}, {0.15f, roof, 1.28f}
    };
    b.addHexahedron(cab, p.hull);

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      addSkirtPanels(b, p, -2.9f, 3.0f, 0.86f, 1.36f, 1.62f, 1.67f, s, 5, 0.2f);

      // Cab side vision blocks and crew door
      for (int k = 0; k < 2; ++k) {
        addVisionBlock(b, p, Point3(1.2f + 0.8f * k, roof - 0.25f, s * 1.31f), Point3(0.0f, 0.0f, s));
      }
      b.addBox(Point3(0.75f, 1.85f, s * 1.36f), 0.32f, 0.38f, 0.015f, p.hull.shade(0.92f));
      b.addBox(Point3(0.5f, 1.85f, s * 1.38f), 0.02f, 0.05f, 0.012f, p.darkMetal);

      // Boom pivot brackets on the cab front corners
      b.addBox(Point3(2.35f, roof + 0.04f, s * 1.15f), 0.18f, 0.08f, 0.1f, p.hullDark);

      // Front headlights, rear lights, tow eyes
      addHeadlight(b, p, Point3(3.12f, 1.48f, s * 1.45f));
      b.addBox(Point3(-3.07f, 1.25f, s * 1.45f), 0.03f, 0.05f, 0.08f, p.tailLight);
      b.addRod(Point3(-3.0f, 0.7f, s * 0.75f), Point3(-3.15f, 0.7f, s * 0.75f), 0.07f, 10, p.darkMetal);

      // Deck stowage along the fenders
      addStowageBox(b, p, Point3(-1.6f, deck + 0.13f, s * 1.38f), 1.0f, 0.13f, 0.2f, p.hullDark);

      // Dozer spade arms
      b.addRod(Point3(2.7f, 0.9f, s * 0.88f), Point3(3.32f, 0.55f, s * 0.88f), 0.07f, 10, p.darkMetal);
      b.addFrustum(Point3(3.0f, 0.73f, s * 0.7f), Point3(0.0f, 0.0f, s), 0.25f, 0.08f, 0.08f, 10, p.steel, true,
                   true);
    }

    // Front windows across the raked cab face
    for (int k = -1; k <= 1; ++k) {
      Point3 center(2.78f, 1.96f, 0.75f * k);
      Point3 normal = Point3(1.02f, 0.6f, 0.0f).normalize();
      Point3 up = Point3(-0.6f, 1.02f, 0.0f).normalize();
      b.addOrientedBox(center, normal, up, Point3(0.0f, 0.0f, 1.0f), 0.02f, 0.12f, 0.22f, p.glass);
    }

    // Dozer spade: lower blade and raked upper blade
    Point3 blade[8] = {
      {3.3f, 0.22f, -1.6f}, {3.48f, 0.22f, -1.6f}, {3.48f, 0.22f, 1.6f}, {3.3f, 0.22f, 1.6f},
      {3.32f, 0.72f, -1.6f}, {3.44f, 0.72f, -1.6f}, {3.44f, 0.72f, 1.6f}, {3.32f, 0.72f, 1.6f}
    };
    b.addHexahedron(blade, p.hull);
    Point3 bladeTop[8] = {
      {3.32f, 0.72f, -1.6f}, {3.44f, 0.72f, -1.6f}, {3.44f, 0.72f, 1.6f}, {3.32f, 0.72f, 1.6f},
      {3.4f, 1.0f, -1.55f}, {3.5f, 1.0f, -1.55f}, {3.5f, 1.0f, 1.55f}, {3.4f, 1.0f, 1.55f}
    };
    b.addHexahedron(bladeTop, p.hullLight);
    b.addBox(Point3(3.47f, 0.2f, 0.0f), 0.04f, 0.03f, 1.62f, p.steel.shade(0.8f));

    // Engine deck grilles, exhaust and the boom rest
    addGrille(b, p, Point3(-2.3f, deck + 0.005f, -0.45f), 0.6f, 0.35f, 10);
    addGrille(b, p, Point3(-2.3f, deck + 0.005f, 0.45f), 0.6f, 0.35f, 10);
    addGrille(b, p, Point3(-0.75f, deck + 0.005f, 0.0f), 0.55f, 0.5f, 8);
    b.addBox(Point3(-3.06f, 0.95f, 0.0f), 0.02f, 0.3f, 0.7f, p.darkMetal);
    for (int k = 0; k < 5; ++k) {
      b.addBox(Point3(-3.085f, 0.75f + 0.1f * k, 0.0f), 0.01f, 0.02f, 0.66f, p.exhaust);
    }
    Point3 restBase(-1.75f, deck, 0.0f);
    float restTop = boomPivot().y - 0.08f;
    b.addBox(Point3(restBase.x, (deck + restTop) * 0.5f, -0.35f), 0.06f, (restTop - deck) * 0.5f, 0.06f,
             p.darkMetal);
    b.addBox(Point3(restBase.x, (deck + restTop) * 0.5f, 0.35f), 0.06f, (restTop - deck) * 0.5f, 0.06f,
             p.darkMetal);
    b.addBox(Point3(restBase.x, restTop - 0.03f, 0.0f), 0.08f, 0.03f, 0.45f, p.darkMetal);

    // Main winch housing with fairlead and spare track links on the rear deck
    b.addBox(Point3(-0.75f, deck + 0.2f, 0.95f), 0.4f, 0.2f, 0.25f, p.hullDark);
    b.addFrustum(Point3(-0.35f, deck + 0.2f, 0.95f), axisX(), 0.08f, 0.12f, 0.1f, 14, p.steel, false, true);
    for (int k = 0; k < 4; ++k) {
      b.addBox(Point3(-2.9f + 0.18f * k, deck + 0.04f, 1.2f), 0.07f, 0.03f, 0.18f, p.track);
    }

    // Tow bar stowed on the rear plate
    b.addRod(Point3(-3.08f, 1.1f, -0.9f), Point3(-3.08f, 1.1f, 0.9f), 0.04f, 8, p.darkMetal);

    // Cab roof: driver hatch, mechanic hatch, antennas
    addHatch(b, p, Point3(2.0f, roof, -0.65f), 0.24f);
    addHatch(b, p, Point3(0.55f, roof, -0.6f), 0.26f);
    addAntenna(b, p, Point3(0.35f, roof, 1.05f), 1.3f);
    addAntenna(b, p, Point3(0.35f, roof, -1.05f), 1.1f);
    b.addBox(Point3(0.5f, roof + 0.08f, 0.6f), 0.18f, 0.08f, 0.2f, p.hullDark);
  }

  /** Commander's cupola with vision blocks and a pintle-mounted .50 cal. */
  void buildTurret(TankMeshBuilder &b) const override {
    using namespace vehicle_parts;
    const TankPalette &p = palette;
    const Point3 Y = axisY();
    b.addFrustum(Point3(0.0f, 0.0f, 0.0f), Y, 0.18f, 0.36f, 0.33f, 24, p.hull, false, true);
    for (int k = 0; k < 6; ++k) {
      float angle = 2.0f * M_PI * static_cast<float>(k) / 6.0f;
      Point3 dir(cos(angle), 0.0f, sin(angle));
      Point3 tangent(-sin(angle), 0.0f, cos(angle));
      b.addOrientedBox(Point3(0.0f, 0.1f, 0.0f) + dir * 0.345f, dir, Y, tangent, 0.03f, 0.04f, 0.06f, p.glass);
    }
    addHatch(b, p, Point3(0.0f, 0.18f, 0.0f), 0.27f);
    b.addRod(Point3(0.25f, 0.18f, 0.0f), Point3(0.25f, 0.42f, 0.0f), 0.025f, 8, p.darkMetal);
    TransformScope mg(b, TankTransform::translation(Point3(0.3f, 0.48f, 0.0f)));
    addHeavyMachineGun(b, p, 0.65f);
  }

  /** A-frame boom in its own frame: origin at the pivot, stowed lying along -X. */
  void buildGun(TankMeshBuilder &b) const override {
    const TankPalette &p = palette;
    const Point3 Z(0.0f, 0.0f, 1.0f);
    const float length = boomLength();
    const float rootZ = 1.15f;
    const float tipZ = 0.3f;

    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      Point3 root(0.0f, 0.0f, s * rootZ);
      Point3 tip(-length, 0.0f, s * tipZ);
      Point3 along = (tip - root).normalize();
      Point3 up(0.0f, 1.0f, 0.0f);
      Point3 side = along.cross(up);
      b.addOrientedBox((root + tip) * 0.5f, along, up, side, length * 0.5f + 0.05f, 0.08f, 0.07f, p.hull);

      // Hydraulic cylinder along each leg
      Point3 c0 = root + along * 0.4f + up * 0.13f;
      Point3 c1 = root + along * 2.2f + up * 0.13f;
      b.addRod(c0, c0 + (c1 - c0) * 0.6f, 0.05f, 10, p.hullDark);
      b.addRod(c0 + (c1 - c0) * 0.6f, c1, 0.03f, 8, p.steel);

      b.addFrustum(Point3(0.0f, 0.0f, s * (rootZ - 0.12f)), Point3(0.0f, 0.0f, s), 0.24f, 0.1f, 0.1f, 14,
                   p.darkMetal, true, true);
    }

    // Cross braces
    for (int k = 1; k <= 2; ++k) {
      float t = static_cast<float>(k) / 3.0f;
      float x = -length * t;
      float z = rootZ + (tipZ - rootZ) * t;
      b.addBox(Point3(x, 0.0f, 0.0f), 0.05f, 0.05f, z, p.hullDark);
    }

    // Boom head with sheaves and the stowed hook block
    b.addBox(Point3(-length - 0.05f, 0.0f, 0.0f), 0.12f, 0.1f, tipZ + 0.06f, p.hullDark);
    for (float s = -1.0f; s <= 1.0f; s += 2.0f) {
      b.addFrustum(Point3(-length - 0.12f, 0.0f, s * 0.08f), Z * s, 0.08f, 0.16f, 0.16f, 18, p.steel, true, true);
    }
    b.addBox(Point3(-length + 0.35f, -0.16f, 0.0f), 0.14f, 0.07f, 0.12f, p.darkMetal);
    b.addFrustum(Point3(-length + 0.35f, -0.23f, 0.0f), Point3(0.0f, -1.0f, 0.0f), 0.08f, 0.04f, 0.02f, 8,
                 p.steel.shade(0.7f), false, true);
    b.addRod(Point3(-length - 0.12f, -0.16f, 0.0f), Point3(-length + 0.25f, -0.16f, 0.0f), 0.015f, 6,
             p.darkMetal);
  }
};

#endif // __M88A3_HPP_
