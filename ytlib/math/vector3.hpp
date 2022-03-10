#pragma once

#include <cmath>
#include <type_traits>

namespace ytlib {

template <std::floating_point TFloat>
class Vector3 {
 public:
  Vector3() {}
  Vector3(TFloat input_x, TFloat input_y, TFloat input_z) : x(input_x), y(input_y), z(input_z) {}
  ~Vector3() {}

  const TFloat Length() const { return std::hypot(x, y, z); }
  const TFloat Distance(const Vector3& rhs) const { return std::hypot(x - rhs.x, y - rhs.y, z - rhs.z); }

  static const TFloat Distance(const Vector3& lhs, const Vector3& rhs) {
    return std::hypot(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
  }

 public:
  TFloat x = 0.0;
  TFloat y = 0.0;
  TFloat z = 0.0;
};

}  // namespace ytlib
