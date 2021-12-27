#pragma once

#include <cinttypes>
#include <cstring>

namespace ytlib {

#if defined(_WIN32)
  #define strcasecmp _stricmp
#endif

/**
 * @brief 日志级别
 */
enum class LOG_LEVEL : uint8_t {
  L_TRACE = 0,
  L_DEBUG,
  L_INFO,
  L_WARN,
  L_ERROR,
  L_FATAL,
  L_NONE,
};

/**
 * @brief 日志级别
 */
class LogLevelName {
 public:
  static const char** GetLogLevelNameArray() {
    static const char* arr[] = {
        "Trace",  // L_TRACE,
        "Debug",  // L_DEBUG,
        "Info",   // L_INFO,
        "Warn",   // L_WARN,
        "Error",  // L_ERROR,
        "Fatal",  // L_FATAL,
        "None",   // L_NONE,
    };
    static_assert(sizeof(arr) / sizeof(arr[0]) == static_cast<uint8_t>(LOG_LEVEL::L_NONE) + 1, "");
    return arr;
  }

  static const char* GetLogLevelName(LOG_LEVEL n) {
    return GetLogLevelNameArray()[static_cast<uint8_t>(n)];
  }

  static LOG_LEVEL GetLogLevelFromName(const char* name) {
    const char** arr = GetLogLevelNameArray();
    for (uint8_t i = 0; i <= static_cast<uint8_t>(LOG_LEVEL::L_NONE); ++i) {
      if (strcmp(name, arr[i]) == 0) return static_cast<LOG_LEVEL>(i);
    }
    return LOG_LEVEL::L_NONE;
  }
};

}  // namespace ytlib
