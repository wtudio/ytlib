#include <gtest/gtest.h>

#include "log.hpp"

using ytlib::Log;

TEST(HelloTest, BasicAssertions) {
  Log::Ins().SetLevel(ytlib::LOG_LEVEL::L_INFO);
  EXPECT_EQ(Log::Ins().Level(), ytlib::LOG_LEVEL::L_INFO);
}
