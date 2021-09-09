#include <gtest/gtest.h>

#include "log.hpp"
#include "log_macro.hpp"

#include "rotate_file_writer.hpp"

namespace ytlib {

TEST(LOG_TEST, log_BASE) {
  Log::Ins().SetLevel(LOG_LEVEL::L_FATAL);
  EXPECT_EQ(Log::Ins().Level(), LOG_LEVEL::L_FATAL);
}

}  // namespace ytlib
