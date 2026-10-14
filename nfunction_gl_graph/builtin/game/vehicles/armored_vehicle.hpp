#ifndef __ARMORED_VEHICLE_HPP_
#define __ARMORED_VEHICLE_HPP_

#include <algorithm>
#include <string>
#include <vector>
#include "vehicle_component.hpp"

/**
 * An armored vehicle is a composite of named parts.
 *   hull    vehicle structure, in vehicle coordinates (+X forward, +Y up, +Z right)
 *   turret  rotated by the turret yaw around +Y at turretMount()
 *   gun     elevated by the gun pitch around +Z at gunMount()
 * The gun is a sibling of the turret. When it is mounted on the turret, its pose is
 * turret yaw then gun pitch, so replacing the turret keeps the gun. A vehicle with no
 * turret mount for the gun (the M88A3 boom) leaves the gun on the hull.
 * Parts are added with add() / addTo(), swapped with replace(), and dropped with remove().
 * replace() keeps the previous mount.
 */
class ArmoredVehicle : public ComplexVehiclePart {
public:
  enum class Part { Hull, Turret, Gun };

  /**
   * Param turretYaw: turret rotation around +Y in radians (0 = gun points along +X)
   * Param gunPitch: gun elevation in radians, clamped to [minGunPitch(), maxGunPitch()]
   */
  ArmoredVehicle(const std::string &vehicleName, float turretYaw, float gunPitch, const TankPalette &palette)
    : ComplexVehiclePart(vehicleName),
      palette(palette),
      turretMountPoint(0.0f, 0.0f, 0.0f),
      gunMountPoint(0.0f, 0.0f, 0.0f),
      turretYaw(turretYaw),
      requestedPitch(gunPitch),
      minPitch(-0.1745f),
      maxPitch(0.349f),
      pitchSign(1.0f),
      gunOnTurret(true) {}

  std::string name() const { return getName(); }

  Point3 turretMount() const { return turretMountPoint; }
  Point3 gunMount() const { return gunMountPoint; }
  float minGunPitch() const { return minPitch; }
  float maxGunPitch() const { return maxPitch; }

  float getTurretYaw() const { return turretYaw; }
  float gunPitch() const {
    return std::max(minPitch, std::min(requestedPitch, maxPitch));
  }
  const TankPalette &getPalette() const { return palette; }

  /** Turret frame expressed in vehicle coordinates. */
  TankTransform turretTransform() const {
    return TankTransform::translation(turretMountPoint) * TankTransform::rotationY(turretYaw);
  }

  /** Gun frame expressed in vehicle coordinates. */
  TankTransform gunTransform() const {
    TankTransform pitched = TankTransform::translation(gunMountPoint) *
                            TankTransform::rotationZ(pitchSign * gunPitch());
    if (gunOnTurret) return turretTransform() * pitched;
    return pitched;
  }

  /**
   * Generate the whole vehicle as one triangle list.
   * Param vertices: output, 6 floats per vertex (x, y, z, nx, ny, nz)
   * Param indices: output, 3 indices per triangle (GL_TRIANGLES)
   * Param colors: output, 4 floats per vertex (r, g, b, a)
   * Return: number of vertices generated
   */
  int generateMesh(std::vector<float> &vertices, std::vector<unsigned short> &indices,
                   std::vector<float> &colors) const {
    TankMeshBuilder b(vertices, indices, colors);
    buildLocal(b);
    return static_cast<int>(b.vertexCount());
  }

  /** Generate one part in its own local frame, including its children. Return: vertex count, or 0. */
  int generatePartMesh(const std::string &partName, std::vector<float> &vertices,
                       std::vector<unsigned short> &indices, std::vector<float> &colors) const {
    const VehiclePart *part = find(partName);
    if (!part) return 0;
    TankMeshBuilder b(vertices, indices, colors);
    part->buildLocalFrame(b);
    return static_cast<int>(b.vertexCount());
  }

  int generatePartMesh(Part part, std::vector<float> &vertices, std::vector<unsigned short> &indices,
                       std::vector<float> &colors) const {
    const char *partName = "hull";
    if (part == Part::Turret) partName = "turret";
    if (part == Part::Gun) partName = "gun";
    return generatePartMesh(partName, vertices, indices, colors);
  }

protected:
  TankPalette palette;

  void setTurretMount(const Point3 &mount) { turretMountPoint = mount; }
  void setGunMount(const Point3 &mount) { gunMountPoint = mount; }
  void setGunPitchLimits(float minP, float maxP) {
    minPitch = minP;
    maxPitch = maxP;
  }
  /** +1 raises the gun with positive pitch. -1 raises a boom that is stowed along -X. */
  void setGunPitchSign(float sign) { pitchSign = sign; }
  /** False when the gun pivots on the hull instead of the turret (M88A3 boom). */
  void setGunOnTurret(bool onTurret) { gunOnTurret = onTurret; }

  void buildLocal(TankMeshBuilder &b) const override {
    layout();
    ComplexVehiclePart::buildLocal(b);
  }

private:
  Point3 turretMountPoint;
  Point3 gunMountPoint;
  float turretYaw;
  float requestedPitch;
  float minPitch;
  float maxPitch;
  float pitchSign;
  bool gunOnTurret;

  void layout() const {
    if (const VehiclePart *turret = find("turret")) {
      turret->setLocalTransform(turretTransform());
    }
    if (const VehiclePart *gun = find("gun")) {
      gun->setLocalTransform(gunTransform());
    }
  }
};

#endif // __ARMORED_VEHICLE_HPP_
