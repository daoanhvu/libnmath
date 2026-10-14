#ifndef __ARMORED_VEHICLE_HPP_
#define __ARMORED_VEHICLE_HPP_

#include <string>
#include "../tank/tank_common.hpp"

/**
 * Base class of the armored vehicles. A vehicle has three rigid parts:
 *   hull   (vehicle coordinates: +X forward, +Y up, +Z right, ground at y = 0)
 *   turret (placed at turretMount(), rotated by the turret yaw around +Y)
 *   gun    (placed at gunMount() in the turret frame, elevated by the gun pitch around +Z)
 * Vehicles without a turret or gun may leave those parts empty or reuse them (e.g. a crane boom).
 */
class ArmoredVehicle {
public:
  enum class Part { Hull, Turret, Gun };

  /**
   * Param turretYaw: turret rotation around +Y in radians (0 = gun points along +X)
   * Param gunPitch: gun elevation in radians, clamped to [minGunPitch(), maxGunPitch()]
   */
  ArmoredVehicle(float turretYaw, float gunPitch, const TankPalette &palette)
    : palette(palette), turretYaw(turretYaw), requestedPitch(gunPitch) {}

  virtual ~ArmoredVehicle() {}

  virtual std::string name() const = 0;
  virtual Point3 turretMount() const = 0;
  virtual Point3 gunMount() const = 0;
  virtual float minGunPitch() const { return -0.1745f; }
  virtual float maxGunPitch() const { return 0.349f; }

  float getTurretYaw() const { return turretYaw; }
  float gunPitch() const {
    return std::max(minGunPitch(), std::min(requestedPitch, maxGunPitch()));
  }
  const TankPalette &getPalette() const { return palette; }

  /** Turret frame expressed in vehicle coordinates. */
  virtual TankTransform turretTransform() const {
    return TankTransform::translation(turretMount()) * TankTransform::rotationY(turretYaw);
  }

  /** Gun frame expressed in vehicle coordinates. */
  virtual TankTransform gunTransform() const {
    return turretTransform() * TankTransform::translation(gunMount()) * TankTransform::rotationZ(gunPitch());
  }

  /**
   * Generate the whole vehicle as one triangle list
   * Param vertices: output, 6 floats per vertex (x, y, z, nx, ny, nz)
   * Param indices: output, 3 indices per triangle (GL_TRIANGLES)
   * Param colors: output, 4 floats per vertex (r, g, b, a)
   * Return: number of vertices generated
   */
  int generateMesh(std::vector<float> &vertices, std::vector<unsigned short> &indices,
                   std::vector<float> &colors) const {
    TankMeshBuilder b(vertices, indices, colors);
    buildHull(b);
    b.transform = turretTransform();
    buildTurret(b);
    b.transform = gunTransform();
    buildGun(b);
    return static_cast<int>(b.vertexCount());
  }

  /** Generate one part alone in its own local frame. */
  int generatePartMesh(Part part, std::vector<float> &vertices, std::vector<unsigned short> &indices,
                       std::vector<float> &colors) const {
    TankMeshBuilder b(vertices, indices, colors);
    switch (part) {
      case Part::Hull: buildHull(b); break;
      case Part::Turret: buildTurret(b); break;
      case Part::Gun: buildGun(b); break;
    }
    return static_cast<int>(b.vertexCount());
  }

protected:
  TankPalette palette;

  virtual void buildHull(TankMeshBuilder &b) const = 0;
  virtual void buildTurret(TankMeshBuilder &b) const = 0;
  virtual void buildGun(TankMeshBuilder &b) const = 0;

private:
  float turretYaw;
  float requestedPitch;
};

#endif // __ARMORED_VEHICLE_HPP_
