#pragma once

#include <memory>
#include <string>
#include "ytlib/math/vector3.hpp"

namespace ytlib {

class Station : public std::enable_shared_from_this<Station> {
 public:
  Station() {}
  virtual ~Station() {}

 public:
  std::string name_;
  uint64_t id_;

  Vector3<float> location_;
};

}  // namespace ytlib
