#pragma once

#include "ytlib/math/vector3.hpp"

namespace ytlib {

class Ship {
 public:
  Ship() {}
  virtual ~Ship() {}

 public:
  Vector3<float> location;
};

}  // namespace ytlib
