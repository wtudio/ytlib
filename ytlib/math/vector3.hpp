#pragma once

#include <cmath>
#include <iostream>
#include <type_traits>

namespace ytlib {

template <std::floating_point TFloat = double>
class Vector3 {
 public:
  Vector3() {}
  Vector3(TFloat input_x, TFloat input_y, TFloat input_z) : x(input_x), y(input_y), z(input_z) {}
  Vector3(const Vector3& value) : x(value.x), y(value.y), z(value.z) {}
  ~Vector3() {}

  const TFloat Len() const { return std::hypot(x, y, z); }
  const TFloat Distance(const Vector3& rhs) const { return std::hypot(x - rhs.x, y - rhs.y, z - rhs.z); }

  static const TFloat Distance(const Vector3& lhs, const Vector3& rhs) {
    return std::hypot(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
  }

  bool operator==(const Vector3& value) const {
    return (std::abs(this->x - value.x) < 1e-6 && std::abs(this->y - value.y) < 1e-6 && std::abs(this->z - value.z) < 1e-6);
  }
  bool operator!=(const Vector3& value) const {
    return !(*this == value);
  }

  Vector3 operator+(const Vector3& value) const {
    return Vector3(this->x + value.x, this->y + value.y, this->z + value.z);
  }
  Vector3& operator+=(const Vector3& value) {
    this->x += value.x;
    this->y += value.y;
    this->z += value.z;
    return *this;
  }

  Vector3 operator-(const Vector3& value) const {
    return Vector3(this->x - value.x, this->y - value.y, this->z - value.z);
  }
  Vector3& operator-=(const Vector3& value) {
    this->x -= value.x;
    this->y -= value.y;
    this->z -= value.z;
    return *this;
  }

  Vector3 operator*(const TFloat& s) const {
    return Vector3(this->x * s, this->y * s, this->z * s);
  }
  Vector3& operator*=(const TFloat& s) {
    this->x *= s;
    this->y *= s;
    this->z *= s;
    return *this;
  }

  Vector3 operator/(const TFloat& s) const {
    return Vector3(this->x / s, this->y / s, this->z / s);
  }
  Vector3& operator/=(const TFloat& s) {
    this->x /= s;
    this->y /= s;
    this->z /= s;
    return *this;
  }

  Vector3 operator-() const {
    return Vector3(-(this->x), -(this->y), -(this->z));
  }

  friend std::ostream& operator<<(std::ostream& output, const Vector3& rhs) {
    output << '(' << rhs.x << ", " << rhs.y << ", " << rhs.z << ')';
    return output;
  }

 public:
  TFloat x = 0.0;
  TFloat y = 0.0;
  TFloat z = 0.0;
};

}  // namespace ytlib
