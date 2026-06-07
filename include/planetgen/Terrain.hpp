#pragma once

#include "planetgen/Math.hpp"
#include "planetgen/Noise.hpp"
#include <memory>

namespace planetgen {

struct TerrainSample {
  double elevation{};
  double continentMask{};
  double mountainMask{};
  double moisture{};
};

struct PlanetSettings {
  double oceanHeight = 0.0;
  double terrainScale = 1.0;
  double continentFrequency = 1.25;
  double mountainFrequency = 6.0;
  double maxContinentHeight = 0.16;
  double maxMountainHeight = 0.09;
  std::uint32_t seed = 42;
};

class ITerrainModule {
public:
  virtual ~ITerrainModule() = default;
  [[nodiscard]] virtual TerrainSample
  sample(const Vec3 &unitDirection) const = 0;
};

class ContinentsTerrain final : public ITerrainModule {
public:
  explicit ContinentsTerrain(PlanetSettings settings);

  [[nodiscard]] TerrainSample sample(const Vec3 &unitDirection) const override;
  [[nodiscard]] const PlanetSettings &settings() const { return settings_; }

private:
  PlanetSettings settings_;
  ValueNoise3D continents_;
  ValueNoise3D mountains_;
  ValueNoise3D moisture_;
};

} // namespace planetgen
