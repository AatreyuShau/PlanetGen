#pragma once

#include "planetgen/Math.hpp"
#include <cstdint>

namespace planetgen {

class ValueNoise3D {
public:
  explicit ValueNoise3D(std::uint32_t seed = 1337);

  [[nodiscard]] double sample(const Vec3 &p) const;
  [[nodiscard]] double fbm(const Vec3 &p, int octaves, double lacunarity,
                           double gain) const;

private:
  std::uint32_t seed_;

  [[nodiscard]] double lattice(int x, int y, int z) const;
};

} // namespace planetgen
