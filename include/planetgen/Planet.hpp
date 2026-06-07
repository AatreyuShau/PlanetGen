#pragma once

#include "planetgen/Terrain.hpp"
#include <memory>

namespace planetgen {

class Planet {
public:
  explicit Planet(PlanetSettings settings = {});
  explicit Planet(std::unique_ptr<ITerrainModule> terrain);

  [[nodiscard]] TerrainSample sample(const Vec3 &unitDirection) const;

  void setTerrain(std::unique_ptr<ITerrainModule> terrain);

private:
  std::unique_ptr<ITerrainModule> terrain_;
};

} // namespace planetgen
