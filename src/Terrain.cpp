#include "planetgen/Terrain.hpp"

namespace planetgen {

ContinentsTerrain::ContinentsTerrain(PlanetSettings settings)
    : settings_(settings), continents_(settings.seed),
      mountains_(settings.seed ^ 0xa511e9b3U),
      moisture_(settings.seed ^ 0x63d83595U) {}

TerrainSample ContinentsTerrain::sample(const Vec3 &unitDirection) const {
  const Vec3 continentPoint = unitDirection * settings_.continentFrequency;
  const double rawContinent = continents_.fbm(continentPoint, 6, 2.02, 0.52);
  const double continentMask = smoothstep(-0.16, 0.34, rawContinent);

  const Vec3 mountainPoint = unitDirection * settings_.mountainFrequency;
  const double ridged =
      1.0 - std::abs(mountains_.fbm(mountainPoint, 5, 2.18, 0.47));
  const double mountainMask = smoothstep(0.52, 0.92, ridged) * continentMask;

  const double shorelineShelf = smoothstep(0.18, 0.38, continentMask);
  const double plains = (continentMask - 0.5) * settings_.maxContinentHeight;
  const double mountains = mountainMask * settings_.maxMountainHeight;
  const double elevation =
      ((plains * shorelineShelf) + mountains) * settings_.terrainScale;

  return {
      elevation,
      continentMask,
      mountainMask,
      (moisture_.fbm(unitDirection * 3.0, 4, 2.0, 0.5) + 1.0) * 0.5,
  };
}

} // namespace planetgen
