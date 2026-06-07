#pragma once

#include <algorithm>
#include <cmath>

namespace planetgen {

struct Vec3 {
  double x{};
  double y{};
  double z{};

  constexpr Vec3() = default;
  constexpr Vec3(double xValue, double yValue, double zValue)
      : x(xValue), y(yValue), z(zValue) {}

  constexpr Vec3 operator+(const Vec3 &other) const {
    return {x + other.x, y + other.y, z + other.z};
  }
  constexpr Vec3 operator-(const Vec3 &other) const {
    return {x - other.x, y - other.y, z - other.z};
  }
  constexpr Vec3 operator*(double scalar) const {
    return {x * scalar, y * scalar, z * scalar};
  }
  constexpr Vec3 operator/(double scalar) const {
    return {x / scalar, y / scalar, z / scalar};
  }
};

inline constexpr double dot(const Vec3 &a, const Vec3 &b) {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline double length(const Vec3 &value) { return std::sqrt(dot(value, value)); }

inline Vec3 normalize(const Vec3 &value) {
  const double len = length(value);
  if (len <= 0.0000001) {
    return {0.0, 1.0, 0.0};
  }
  return value / len;
}

inline double clamp(double value, double low, double high) {
  return std::max(low, std::min(value, high));
}

inline double smoothstep(double edge0, double edge1, double x) {
  const double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return t * t * (3.0 - (2.0 * t));
}

inline Vec3 rotateYawPitch(const Vec3 &value, double yaw, double pitch) {
  const double cy = std::cos(yaw);
  const double sy = std::sin(yaw);
  const double cp = std::cos(pitch);
  const double sp = std::sin(pitch);

  const Vec3 yawed{
      (value.x * cy) + (value.z * sy),
      value.y,
      (-value.x * sy) + (value.z * cy),
  };

  return {
      yawed.x,
      (yawed.y * cp) - (yawed.z * sp),
      (yawed.y * sp) + (yawed.z * cp),
  };
}

} // namespace planetgen
