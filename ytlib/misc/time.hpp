/**
 * @file time.hpp
 * @author WT
 * @brief 时间相关工具
 * @date 2021-11-03
 */
#pragma once

#include <chrono>
#include <ctime>
#include <string>

#define SECOND_PER_MINUTE (60)
#define MINUTE_PER_HOUR (60)
#define SECOND_PER_HOUR (3600)
#define HOUR_PER_DAY (24)
#define SECOND_PER_DAY (86400)
#define MS_PER_SECOND (1000)
#define US_PER_MS (1000)
#define US_PER_SECOND (1000000)
#define DAY_PER_MONTH (30)
#define DAY_PER_WEEK (7)
#define MONTH_PER_YEAR (12)

namespace ytlib {

/**
 * @brief 获取毫秒时间戳
 *
 * @param t 时间点
 * @return uint64_t 毫秒时间戳
 */
inline uint64_t GetTimestampMs(const std::chrono::system_clock::time_point& t) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count();
}

/**
 * @brief 获取当前时间毫秒时间戳
 *
 * @return uint64_t 当前时间毫秒时间戳
 */
inline uint64_t GetCurTimestampMs() {
  return GetTimestampMs(std::chrono::system_clock::now());
}

/**
 * @brief 获取秒时间戳
 *
 * @param t 时间点
 * @return uint64_t 秒时间戳
 */
inline uint64_t GetTimestampSec(const std::chrono::system_clock::time_point& t) {
  return std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
}

/**
 * @brief 获取当前时间秒时间戳
 *
 * @return uint64_t 当前时间秒时间戳
 */
inline uint64_t GetCurTimestampSec() {
  return GetTimestampSec(std::chrono::system_clock::now());
}

/**
 * @brief 获取当前时间
 *
 * @return time_t
 */
inline time_t GetCurTimeT() {
  return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

/**
 * @brief localtime
 *
 * @param t
 * @return struct tm
 */
inline struct tm TimeT2TmLocal(const time_t& t) {
  struct tm st;
#if defined(_WIN32)
  localtime_s(&st, &t);
#else
  localtime_r(&t, &st);
#endif
  return st;
}

/**
 * @brief gmtime
 *
 * @param t
 * @return struct tm
 */
inline struct tm TimeT2TmGm(const time_t& t) {
  struct tm st;
#if defined(_WIN32)
  gmtime_s(&st, &t);
#else
  gmtime_r(&t, &st);
#endif
  return st;
}

/**
 * @brief 时间转字符串
 *
 * @param t
 * @return std::string
 */
inline std::string GetTimeStr(tm t) {
  char buf[16];  // 4+2+2+1+2+2+2+1
  snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d",
           t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  return buf;
}

/**
 * @brief 时间转字符串
 *
 * @param t
 * @return std::string
 */
inline std::string GetTimeStr(time_t t) {
  return GetTimeStr(TimeT2TmLocal(t));
}

/**
 * @brief 获取当前时间
 *
 * @return struct tm
 */
inline struct tm GetCurTm() {
  return TimeT2TmLocal(GetCurTimeT());
}

/**
 * @brief 获取当前时间字符串
 *
 * @return std::string
 */
inline std::string GetCurTimeStr() {
  return GetTimeStr(GetCurTimeT());
}

/**
 * @brief 判断是否为闰年
 *
 * @param year
 * @return true
 * @return false
 */
inline bool IsLeapYear(uint32_t year) {
  return (year % 100) ? (year % 4 == 0) : (year % 400 == 0);
}

/**
 * @brief 获取当月天数
 *
 * @param year 年份
 * @param month 从0开始，0代表1月
 * @return uint32_t
 */
inline uint32_t GetMonthDayCount(uint32_t year, uint32_t month) {
  const static uint32_t month_day_count[] =
      {
          31,  // Jan
          28,  // Feb
          31,  // Mar
          30,  // Apr
          31,  // May
          30,  // Jun
          31,  // Jul
          31,  // Aug
          30,  // Sep
          31,  // Oct
          30,  // Nov
          31,  // Dec
      };
  return (month == 1 && IsLeapYear(year)) ? 29 : month_day_count[month];
}

/**
 * @brief 获取所在时区
 *
 * @return int32_t 时区，单位s
 */
inline int32_t GetLocalTimeZone() {
  time_t now = GetCurTimeT();
  struct tm gt, lt;

#if defined(_WIN32)
  gmtime_s(&gt, &now);
  localtime_s(&lt, &now);
#else
  gmtime_r(&now, &gt);
  localtime_r(&now, &lt);
#endif

  time_t gmt = mktime(&gt);
  time_t lmt = mktime(&lt);

  return (int32_t)difftime(lmt, gmt);
}

/**
 * @brief 判断l_time是否在r_time后一天
 *
 * @param l_time
 * @param r_time
 * @param time_zone 时区，单位s
 * @return true
 * @return false
 */
inline bool IsPassDay(time_t l_time, time_t r_time, int32_t time_zone) {
  time_t l_day = static_cast<time_t>((l_time + time_zone) / SECOND_PER_DAY);
  time_t r_day = static_cast<time_t>((r_time + time_zone) / SECOND_PER_DAY);
  return (l_day > r_day);
}

/**
 * @brief 获取一天开始的时间点
 *
 * @param t
 * @return time_t
 */
inline time_t GetDayStartTime(time_t t) {
  struct tm st = TimeT2TmLocal(t);

  st.tm_sec = 0;
  st.tm_min = 0;
  st.tm_hour = 0;

  return mktime(&st);
}

/**
 * @brief 获取星期几，1~7
 *
 * @param t
 * @return uint32_t 1~7
 */
inline uint32_t GetWeekDay(time_t t) {
  struct tm st = TimeT2TmLocal(t);
  return (st.tm_wday) ? (st.tm_wday) : 7;
}

/**
 * @brief 获取当周周一的开始时间点
 *
 * @param t
 * @return uint32_t
 */
inline uint32_t GetWeekStartTime(time_t t) {
  return GetDayStartTime(t - (GetWeekDay(t) - 1) * SECOND_PER_DAY);
}

/**
 * @brief 计算两个时间之间日期差
 *
 * @param l_time
 * @param r_time
 * @param time_zone
 * @return int32_t
 */
inline int32_t GetDayCount(time_t l_time, time_t r_time, int32_t time_zone) {
  return (l_time + time_zone - r_time) / SECOND_PER_DAY;
}

}  // namespace ytlib
