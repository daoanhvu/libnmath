#ifndef __US_VEHICLES_HPP_
#define __US_VEHICLES_HPP_

#include <cstring>
#include <memory>
#include "abrams.hpp"
#include "m10_booker.hpp"
#include "m88a3.hpp"
#include "ampv.hpp"
#include "bradley.hpp"

/** The vehicles of the "Latest U.S. Tanks" poster. */
enum class UsVehicleType {
  M1A2_SEPV3,
  M1A2_SEPV4,
  M10_BOOKER,
  M88A3,
  M1A2D,
  MPF,
  AMPV,
  M3A3E1_BRADLEY
};

struct UsVehicleInfo {
  UsVehicleType type;
  /** Command name typed in the function box, e.g. sepv3(30, 5) */
  const char *command;
  const char *displayName;
};

inline const std::vector<UsVehicleInfo> &usVehicleCatalog() {
  static const std::vector<UsVehicleInfo> catalog = {
    {UsVehicleType::M1A2_SEPV3, "sepv3", "M1A2 SEPv3 Abrams"},
    {UsVehicleType::M1A2_SEPV4, "sepv4", "M1A2 SEPv4 Abrams"},
    {UsVehicleType::M10_BOOKER, "booker", "M10 Booker"},
    {UsVehicleType::M88A3, "m88", "M88A3 Recovery Vehicle"},
    {UsVehicleType::M1A2D, "m1a2d", "M1A2D Abrams"},
    {UsVehicleType::MPF, "mpf", "Mobile Protected Firepower"},
    {UsVehicleType::AMPV, "ampv", "AMPV"},
    {UsVehicleType::M3A3E1_BRADLEY, "bradley", "M3A3E1 Bradley"}
  };
  return catalog;
}

/** Return: catalog entry whose command equals name, or nullptr */
inline const UsVehicleInfo *findUsVehicleByCommand(const char *name) {
  for (const UsVehicleInfo &info : usVehicleCatalog()) {
    if (std::strcmp(info.command, name) == 0) {
      return &info;
    }
  }
  return nullptr;
}

/**
 * Param turretYaw: radians around +Y (cupola for the M88A3)
 * Param gunPitch: radians (boom raise for the M88A3), clamped by each vehicle
 */
inline std::unique_ptr<ArmoredVehicle> createUsVehicle(UsVehicleType type, float turretYaw = 0.0f,
                                                       float gunPitch = 0.0f) {
  switch (type) {
    case UsVehicleType::M1A2_SEPV3:
      return std::unique_ptr<ArmoredVehicle>(new Abrams(AbramsVariant::M1A2_SEPV3, turretYaw, gunPitch));
    case UsVehicleType::M1A2_SEPV4:
      return std::unique_ptr<ArmoredVehicle>(new Abrams(AbramsVariant::M1A2_SEPV4, turretYaw, gunPitch));
    case UsVehicleType::M10_BOOKER:
      return std::unique_ptr<ArmoredVehicle>(new M10Booker(BookerVariant::M10_BOOKER, turretYaw, gunPitch));
    case UsVehicleType::M88A3:
      return std::unique_ptr<ArmoredVehicle>(new M88A3(turretYaw, gunPitch));
    case UsVehicleType::M1A2D:
      return std::unique_ptr<ArmoredVehicle>(new Abrams(AbramsVariant::M1A2D, turretYaw, gunPitch));
    case UsVehicleType::MPF:
      return std::unique_ptr<ArmoredVehicle>(new M10Booker(BookerVariant::MPF_PROTOTYPE, turretYaw, gunPitch));
    case UsVehicleType::AMPV:
      return std::unique_ptr<ArmoredVehicle>(new Ampv(turretYaw, gunPitch));
    case UsVehicleType::M3A3E1_BRADLEY:
      return std::unique_ptr<ArmoredVehicle>(new Bradley(turretYaw, gunPitch));
  }
  return nullptr;
}

#endif // __US_VEHICLES_HPP_
