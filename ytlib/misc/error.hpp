#pragma once

#include <exception>
#include <string>

#include "misc_macro.h"

namespace ytlib {

/**
 * @brief 自定义异常类
 * 重载了std::exception
 */
class Exception : public std::exception {
 public:
  Exception(const char* msg) : msg_(msg) {}

  virtual ~Exception() throw() {}

  virtual const char* what() const throw() {
    return msg_;
  }

 protected:
  const char* msg_;
};

// runtime assert
#define RT_ASSERT(exp, ...) \
  if (!(exp)) [[unlikely]]  \
    throw ytlib::Exception("[" __FILE__ ":" STRING(__LINE__) "] assert ( " #exp " ) failed. " __VA_ARGS__);

}  // namespace ytlib
