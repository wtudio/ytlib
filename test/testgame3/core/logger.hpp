/**
 * @file logger.hpp
 * @brief 双通道日志：事件流 + 周期快照，均落盘
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <string>

#include "core/types.hpp"

namespace ytlib::testgame3 {

class Logger {
 public:
  Logger(const std::string& event_path, const std::string& snapshot_path,
         const std::string& init_path)
      : event_(event_path, std::ios::out | std::ios::trunc),
        snapshot_(snapshot_path, std::ios::out | std::ios::trunc),
        init_(init_path, std::ios::out | std::ios::trunc) {}

  void Event(GameTime t, const std::string& msg) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "[t=%08llu] ", static_cast<unsigned long long>(t));
    event_ << buf << msg << "\n";
  }

  void Eventf(GameTime t, const char* fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    Event(t, buf);
  }

  std::ofstream& SnapshotStream() { return snapshot_; }
  std::ofstream& InitStream() { return init_; }

  void SnapshotHeader(GameTime t) {
    char buf[64];
    std::snprintf(buf, sizeof(buf),
                  "\n==================== t=%08llu ====================\n",
                  static_cast<unsigned long long>(t));
    snapshot_ << buf;
  }

  void Flush() {
    event_.flush();
    snapshot_.flush();
    init_.flush();
  }

 private:
  std::ofstream event_;
  std::ofstream snapshot_;
  std::ofstream init_;
};

}  // namespace ytlib::testgame3
