#ifndef __TANK_HPP_
#define __TANK_HPP_

#include "tank_common.hpp"
#include "tank_hull.hpp"
#include "tank_turret.hpp"
#include "tank_gun.hpp"

/**
 * Complete tank assembled from its parts:
 *   hull   (hull coordinates = tank coordinates, tracks on y = 0)
 *   turret (placed at TankHull::turretMount(), rotated by turretYaw around +Y)
 *   gun    (placed at TankTurret::gunMount(), elevated by gunPitch around +Z)
 *
 * Each part can also be generated alone in its own frame with generateTankPartMesh(),
 * then positioned with turretTransform() and gunTransform().
 */
class Tank {
public:
  /**
   * Param turretYaw: turret rotation around +Y in radians (0 = cannon points along +X)
   * Param gunPitch: cannon elevation in radians, clamped to [TankGun::minPitch(), TankGun::maxPitch()]
   * Param palette: colors, TankPalette::fromHullColor() derives the shades from one base color
   */
  Tank(float turretYaw = 0.0f, float gunPitch = 0.0f,
       const TankPalette &palette = TankPalette::defaultPalette())
    : hull(palette), turret(palette), gun(palette),
      turretYaw(turretYaw),
      gunPitch(std::max(TankGun::minPitch(), std::min(gunPitch, TankGun::maxPitch()))) {}

  /** Turret frame expressed in tank coordinates. */
  TankTransform turretTransform() const {
    return TankTransform::translation(TankHull::turretMount()) * TankTransform::rotationY(turretYaw);
  }

  /** Gun frame expressed in tank coordinates. */
  TankTransform gunTransform() const {
    return turretTransform() * TankTransform::translation(TankTurret::gunMount()) *
           TankTransform::rotationZ(gunPitch);
  }

  /**
   * Generate the whole tank as one triangle list
   * Param vertices: output, 6 floats per vertex (x, y, z, nx, ny, nz)
   * Param indices: output, 3 indices per triangle (GL_TRIANGLES)
   * Param colors: output, 4 floats per vertex (r, g, b, a)
   * Return: number of vertices generated
   */
  int generateMesh(std::vector<float> &vertices, std::vector<unsigned short> &indices,
                   std::vector<float> &colors) const {
    TankMeshBuilder b(vertices, indices, colors);
    hull.build(b);
    b.transform = turretTransform();
    turret.build(b);
    b.transform = gunTransform();
    gun.build(b);
    return static_cast<int>(b.vertexCount());
  }

  const TankHull &getHull() const { return hull; }
  const TankTurret &getTurret() const { return turret; }
  const TankGun &getGun() const { return gun; }

private:
  TankHull hull;
  TankTurret turret;
  TankGun gun;
  float turretYaw;
  float gunPitch;
};

#endif // __TANK_HPP_
