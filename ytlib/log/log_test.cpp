#include "log.hpp"

#include <gtest/gtest.h>

namespace ytlib {

TEST(LOG_TEST, LEVEL) {
  Log::Ins().SetLevel(LOG_LEVEL::L_FATAL);
  EXPECT_EQ(Log::Ins().Level(), LOG_LEVEL::L_FATAL);
}

}  // namespace ytlib
