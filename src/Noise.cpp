#include "planetgen/Noise.hpp"

#include <cmath>

namespace planetgen {
namespace {

double fade(double t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

double lerp(double a, double b, double t) { return a + ((b - a) * t); }

std::uint32_t hash(std::uint32_t x) {
  x ^= x >> 16U;
  x *= 0x7feb352dU;
  x ^= x >> 15U;
  x *= 0x846ca68bU;
  x ^= x >> 16U;
  return x;
}

} // namespace

ValueNoise3D::ValueNoise3D(std::uint32_t seed) : seed_(seed) {}

double ValueNoise3D::lattice(int x, int y, int z) const {
  std::uint32_t h = seed_;
  h ^= hash(static_cast<std::uint32_t>(x) + 0x9e3779b9U);
  h ^=
      hash(static_cast<std::uint32_t>(y) + 0x85ebca6bU + (h << 6U) + (h >> 2U));
  h ^=
      hash(static_cast<std::uint32_t>(z) + 0xc2b2ae35U + (h << 6U) + (h >> 2U));
  return (static_cast<double>(hash(h)) / static_cast<double>(UINT32_MAX)) *
             2.0 -
         1.0;
}

double ValueNoise3D::sample(const Vec3 &p) const {
  const int x0 = static_cast<int>(std::floor(p.x));
  const int y0 = static_cast<int>(std::floor(p.y));
  const int z0 = static_cast<int>(std::floor(p.z));

  const double tx = fade(p.x - static_cast<double>(x0));
  const double ty = fade(p.y - static_cast<double>(y0));
  const double tz = fade(p.z - static_cast<double>(z0));

  const double c000 = lattice(x0, y0, z0);
  const double c100 = lattice(x0 + 1, y0, z0);
  const double c010 = lattice(x0, y0 + 1, z0);
  const double c110 = lattice(x0 + 1, y0 + 1, z0);
  const double c001 = lattice(x0, y0, z0 + 1);
  const double c101 = lattice(x0 + 1, y0, z0 + 1);
  const double c011 = lattice(x0, y0 + 1, z0 + 1);
  const double c111 = lattice(x0 + 1, y0 + 1, z0 + 1);

  const double x00 = lerp(c000, c100, tx);
  const double x10 = lerp(c010, c110, tx);
  const double x01 = lerp(c001, c101, tx);
  const double x11 = lerp(c011, c111, tx);
  const double y0v = lerp(x00, x10, ty);
  const double y1v = lerp(x01, x11, ty);
  return lerp(y0v, y1v, tz);
}

double ValueNoise3D::fbm(const Vec3 &p, int octaves, double lacunarity,
                         double gain) const {
  double amplitude = 0.5;
  double frequency = 1.0;
  double total = 0.0;
  double normalization = 0.0;

  for (int i = 0; i < octaves; ++i) {
    total += sample(p * frequency) * amplitude;
    normalization += amplitude;
    frequency *= lacunarity;
    amplitude *= gain;
  }

  if (normalization <= 0.0) {
    return 0.0;
  }
  return total / normalization;
}

} // namespace planetgen
