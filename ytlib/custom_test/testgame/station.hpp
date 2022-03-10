#pragma once

#include "ytlib/math/vector3.hpp"

namespace ytlib {

class Station {
 public:
  Station() {}
  virtual ~Station() {}

 public:
  Vector3<float> location;
};

}  // namespace ytlib
