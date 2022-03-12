#pragma once

#include <memory>
#include <string>

namespace ytlib {

class Group : public std::enable_shared_from_this<Group> {
 public:
  Group() {}
  virtual ~Group() {}

 public:
  std::string name_;
  uint64_t id_;

  uint64_t money_;
};

}  // namespace ytlib
