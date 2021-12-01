#include <gtest/gtest.h>

#include "time.hpp"

namespace ytlib {

TEST(TIME_TEST, BASE_test) {
  std::string s = GetCurTimeStr();
  ASSERT_EQ(s.length(), 15);
}

}  // namespace ytlib
