#include "planetgen/Planet.hpp"

namespace planetgen {

Planet::Planet(PlanetSettings settings)
    : terrain_(std::make_unique<ContinentsTerrain>(settings)) {}

Planet::Planet(std::unique_ptr<ITerrainModule> terrain)
    : terrain_(std::move(terrain)) {}

TerrainSample Planet::sample(const Vec3 &unitDirection) const {
  return terrain_->sample(unitDirection);
}

void Planet::setTerrain(std::unique_ptr<ITerrainModule> terrain) {
  terrain_ = std::move(terrain);
}

} // namespace planetgen
