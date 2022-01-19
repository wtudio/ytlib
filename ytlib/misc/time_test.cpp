#include <gtest/gtest.h>

#include "time.hpp"

namespace ytlib {

TEST(TIME_TEST, BASE_test) {
  std::string s = GetCurTimeStr();
  EXPECT_EQ(s.length(), 15);

  EXPECT_EQ(IsLeapYear(2000), true);
  EXPECT_EQ(IsLeapYear(2001), false);
  EXPECT_EQ(IsLeapYear(2004), true);
  EXPECT_EQ(IsLeapYear(1900), false);

  EXPECT_EQ(GetMonthDayCount(2000, 0), 31);
  EXPECT_EQ(GetMonthDayCount(2000, 1), 29);
  EXPECT_EQ(GetMonthDayCount(2000, 2), 31);

  // 北京时间是东8区，仅东8区此用例成立
  // EXPECT_EQ(GetLocalTimeZone(), 8 * 3600);

  time_t t = 1642582591;  // 2022-01-19 16:56:31
  EXPECT_EQ(IsPassDay(t + 86400, t, 0), true);
  EXPECT_EQ(GetDayStartTime(t), 1642521600);
  EXPECT_EQ(GetWeekDay(t), 3);
  EXPECT_EQ(GetWeekStartTime(t), 1642348800);

  EXPECT_EQ(GetDayCount(t, t + 3600 * 70, 0), -2);
}

}  // namespace ytlib
