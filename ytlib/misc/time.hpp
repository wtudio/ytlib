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

namespace ytlib {

inline time_t GetCurTimeT() {
  return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

inline struct tm TimeT2Tm(const time_t& t) {
  struct tm st;
#if defined(_WIN32)
  localtime_s(&st, &t);
#else
  localtime_r(&t, &st);
#endif
  return st;
}

inline struct tm GetCurTm() {
  return TimeT2Tm(GetCurTimeT());
}

inline std::string GetCurTimeStr() {
  struct tm t = GetCurTm();
  char buf[16];  // 4+2+2+1+2+2+2+1
  snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d",
           t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  return buf;
}

}  // namespace ytlib
