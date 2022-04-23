#pragma once

#include <memory>
#include <string>

namespace ytlib {

class Employee : public std::enable_shared_from_this<Employee> {
 public:
  Employee() {}
  virtual ~Employee() {}

 public:
  std::string name_;
  uint64_t id_;
};

}  // namespace ytlib
