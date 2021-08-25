#pragma once

#include <exception>
#include <string>

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

#define _ASSERT_PRINT_STRING(x) #x
#define ASSERT_PRINT_STRING(x) _ASSERT_PRINT_STRING(x)

// runtime assert
#define RT_ASSERT(exp, ...) \
  if (!(exp)) [[unlikely]]  \
    throw ytlib::Exception("[" __FILE__ ":" ASSERT_PRINT_STRING(__LINE__) "] assert ( " #exp " ) failed. " __VA_ARGS__);

}  // namespace ytlib
