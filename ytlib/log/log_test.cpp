#include <gtest/gtest.h>

#include "log.hpp"

using ytlib::Log;

TEST(LOG_TEST, LEVEL) {
  Log::Ins().SetLevel(ytlib::LOG_LEVEL::L_FATAL);
  EXPECT_EQ(Log::Ins().Level(), ytlib::LOG_LEVEL::L_FATAL);
}
