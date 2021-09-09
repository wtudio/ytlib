#include <gtest/gtest.h>

#include "dynamic_lib.hpp"

namespace ytlib {

TEST(MISC_TEST, DynamicLib_BASE) {
  DynamicLib d;
  ASSERT_FALSE(d);
  ASSERT_STREQ(d.GetLibName().c_str(), "");
  ASSERT_FALSE(d.Load("xxx/xxx"));

  ASSERT_FALSE(DynamicLibContainer::Ins().LoadLib("xxx/xxx"));
  ASSERT_FALSE(DynamicLibContainer::Ins().GetLib("xxx/xxx"));
  ASSERT_FALSE(DynamicLibContainer::Ins().RemoveLib("xxx/xxx"));
}

}  // namespace ytlib
