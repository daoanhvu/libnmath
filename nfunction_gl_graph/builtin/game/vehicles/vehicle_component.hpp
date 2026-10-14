#ifndef __VEHICLE_COMPONENT_HPP_
#define __VEHICLE_COMPONENT_HPP_

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "../tank/tank_running_gear.hpp"
#include "vehicle_parts.hpp"

/**
 * Composite parts for an armored vehicle, in the same shape as the VBO hierarchy:
 *   VehiclePart         - one node (VBO)
 *   SimpleVehiclePart   - one mesh (SimpleVbo)
 *   ComplexVehiclePart  - a named list of child parts (ComplexVbo)
 *
 * A part's local transform is its mount in the parent frame. Replacing a part keeps
 * that mount, so an upgrade occupies the same place. build() draws this part and,
 * for a group, each child. buildLocalFrame() draws the part without its own mount.
 */
class VehiclePart {
public:
  explicit VehiclePart(std::string partName) : name(std::move(partName)), local(TankTransform::identity()) {}
  virtual ~VehiclePart() = default;

  const std::string &getName() const { return name; }
  void setName(const std::string &value) { name = value; }

  const TankTransform &getLocalTransform() const { return local; }
  void setLocalTransform(const TankTransform &value) const { local = value; }

  /** This part, or a nested part with this name. */
  virtual const VehiclePart *find(const std::string &partName) const {
    return partName == name ? this : nullptr;
  }
  virtual VehiclePart *find(const std::string &partName) {
    return const_cast<VehiclePart *>(static_cast<const VehiclePart *>(this)->find(partName));
  }

  /** Direct children. Leaves have none. */
  virtual int countComponents() const { return 0; }

  bool add(std::unique_ptr<VehiclePart> part) {
    if (!part) return false;
    return addExisting(part);
  }

  /** Swap a nested part. The new part keeps the previous name and mount. */
  bool replace(const std::string &partName, std::unique_ptr<VehiclePart> part) {
    if (!part) return false;
    return replaceExisting(partName, part);
  }

  /** Append a part under the group with this name. */
  bool addTo(const std::string &parentName, std::unique_ptr<VehiclePart> part) {
    if (!part) return false;
    return addToExisting(parentName, part);
  }

  bool remove(const std::string &partName) {
    return removeExisting(partName);
  }

  void collectPartNames(std::vector<std::string> &names) const {
    names.push_back(name);
    collectChildNames(names);
  }

  void build(TankMeshBuilder &b) const {
    vehicle_parts::TransformScope scope(b, local);
    buildLocal(b);
  }

  void buildLocalFrame(TankMeshBuilder &b) const { buildLocal(b); }

  virtual bool addExisting(std::unique_ptr<VehiclePart> &) { return false; }
  virtual bool replaceExisting(const std::string &, std::unique_ptr<VehiclePart> &) { return false; }
  virtual bool addToExisting(const std::string &, std::unique_ptr<VehiclePart> &) { return false; }
  virtual bool removeExisting(const std::string &) { return false; }

protected:
  virtual void buildLocal(TankMeshBuilder &b) const = 0;
  virtual void collectChildNames(std::vector<std::string> &) const {}

  std::string name;
  mutable TankTransform local;
};

/** One mesh. The build function draws in this part's local frame. */
class SimpleVehiclePart : public VehiclePart {
public:
  explicit SimpleVehiclePart(const std::string &partName, std::function<void(TankMeshBuilder &)> draw)
    : VehiclePart(partName), draw(std::move(draw)) {}

protected:
  void buildLocal(TankMeshBuilder &b) const override { draw(b); }

private:
  std::function<void(TankMeshBuilder &)> draw;
};

template <typename Fn>
std::unique_ptr<VehiclePart> makeMeshPart(const std::string &partName, Fn draw) {
  return std::unique_ptr<VehiclePart>(new SimpleVehiclePart(
      partName, std::function<void(TankMeshBuilder &)>(std::move(draw))));
}

/** A named list of child parts. Children are drawn in the order they were added. */
class ComplexVehiclePart : public VehiclePart {
public:
  explicit ComplexVehiclePart(const std::string &partName) : VehiclePart(partName) {}

  int countComponents() const override { return static_cast<int>(components.size()); }

  const std::vector<std::unique_ptr<VehiclePart>> &getComponents() const { return components; }

  const VehiclePart *find(const std::string &partName) const override {
    if (partName == name) return this;
    for (const std::unique_ptr<VehiclePart> &child : components) {
      if (const VehiclePart *found = child->find(partName)) return found;
    }
    return nullptr;
  }

protected:
  void buildLocal(TankMeshBuilder &b) const override {
    for (const std::unique_ptr<VehiclePart> &child : components) {
      child->build(b);
    }
  }

  void collectChildNames(std::vector<std::string> &names) const override {
    for (const std::unique_ptr<VehiclePart> &child : components) {
      child->collectPartNames(names);
    }
  }

  bool addExisting(std::unique_ptr<VehiclePart> &part) override {
    components.push_back(std::move(part));
    return true;
  }

  bool replaceExisting(const std::string &partName, std::unique_ptr<VehiclePart> &part) override {
    for (std::unique_ptr<VehiclePart> &child : components) {
      if (child->getName() == partName) {
        part->setName(child->getName());
        part->setLocalTransform(child->getLocalTransform());
        child = std::move(part);
        return true;
      }
    }
    for (std::unique_ptr<VehiclePart> &child : components) {
      if (child->replaceExisting(partName, part)) return true;
    }
    return false;
  }

  bool addToExisting(const std::string &parentName, std::unique_ptr<VehiclePart> &part) override {
    if (name == parentName) {
      components.push_back(std::move(part));
      return true;
    }
    for (std::unique_ptr<VehiclePart> &child : components) {
      if (child->addToExisting(parentName, part)) return true;
    }
    return false;
  }

  bool removeExisting(const std::string &partName) override {
    for (auto it = components.begin(); it != components.end(); ++it) {
      if ((*it)->getName() == partName) {
        components.erase(it);
        return true;
      }
    }
    for (std::unique_ptr<VehiclePart> &child : components) {
      if (child->removeExisting(partName)) return true;
    }
    return false;
  }

private:
  std::vector<std::unique_ptr<VehiclePart>> components;
};

/** Main gun from a GunSpec, barrel along +X from the trunnion. */
class GunPart : public VehiclePart {
public:
  GunPart(const std::string &partName, const TankPalette &palette, const vehicle_parts::GunSpec &spec)
    : VehiclePart(partName), palette(palette), spec(spec) {}

protected:
  void buildLocal(TankMeshBuilder &b) const override {
    vehicle_parts::addGun(b, palette, spec);
  }

private:
  TankPalette palette;
  vehicle_parts::GunSpec spec;
};

/** Pintle machine gun along +X from its mount. */
class MachineGunPart : public VehiclePart {
public:
  enum class Kind { Medium, Heavy };

  MachineGunPart(const std::string &partName, const TankPalette &palette, Kind kind, float barrelLength)
    : VehiclePart(partName), palette(palette), kind(kind), barrelLength(barrelLength) {}

protected:
  void buildLocal(TankMeshBuilder &b) const override {
    if (kind == Kind::Heavy) {
      vehicle_parts::addHeavyMachineGun(b, palette, barrelLength);
    } else {
      vehicle_parts::addMachineGun(b, palette, barrelLength);
    }
  }

private:
  TankPalette palette;
  Kind kind;
  float barrelLength;
};

class CrowsBasePart : public VehiclePart {
public:
  CrowsBasePart(const std::string &partName, const TankPalette &palette)
    : VehiclePart(partName), palette(palette) {}

protected:
  void buildLocal(TankMeshBuilder &b) const override {
    vehicle_parts::addCrowsBase(b, palette);
  }

private:
  TankPalette palette;
};

class CrowsCradlePart : public VehiclePart {
public:
  CrowsCradlePart(const std::string &partName, const TankPalette &palette)
    : VehiclePart(partName), palette(palette) {}

protected:
  void buildLocal(TankMeshBuilder &b) const override {
    vehicle_parts::addCrowsCradle(b, palette);
  }

private:
  TankPalette palette;
};

/** CROWS station with the cradle fixed at the elevation pivot (pitch is applied by a gun part). */
inline std::unique_ptr<ComplexVehiclePart> makeCrows(const std::string &partName, const TankPalette &palette) {
  std::unique_ptr<ComplexVehiclePart> station(new ComplexVehiclePart(partName));
  station->add(std::unique_ptr<VehiclePart>(new CrowsBasePart(partName + ".base", palette)));
  std::unique_ptr<VehiclePart> cradle(new CrowsCradlePart(partName + ".cradle", palette));
  cradle->setLocalTransform(TankTransform::translation(vehicle_parts::crowsElevationPivot()));
  station->add(std::move(cradle));
  return station;
}

/** Trophy APS unit. side is +1 (right) or -1 (left). Origin is the turret attachment point. */
class TrophyPart : public VehiclePart {
public:
  TrophyPart(const std::string &partName, const TankPalette &palette, float side)
    : VehiclePart(partName), palette(palette), side(side) {}

protected:
  void buildLocal(TankMeshBuilder &b) const override {
    vehicle_parts::addTrophy(b, palette, side);
  }

private:
  TankPalette palette;
  float side;
};

/** Both sides of a tracked running gear. */
class TrackedRunningGearPart : public VehiclePart {
public:
  TrackedRunningGearPart(const std::string &partName, const TankPalette &palette, const TrackedRunningGearSpec &spec)
    : VehiclePart(partName), palette(palette), spec(spec) {}

protected:
  void buildLocal(TankMeshBuilder &b) const override {
    TrackedRunningGear gear(spec, palette);
    gear.build(b, 1.0f);
    gear.build(b, -1.0f);
  }

private:
  TankPalette palette;
  TrackedRunningGearSpec spec;
};

#endif // __VEHICLE_COMPONENT_HPP_
