#pragma once

#include <memory>
#include <string>

namespace ytlib {

class Goods : public std::enable_shared_from_this<Goods> {
 public:
  Goods() {}
  virtual ~Goods() {}

 public:
  std::string type_name_;
  uint64_t type_id_;
};

}  // namespace ytlib
