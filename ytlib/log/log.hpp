#pragma once

#include <cinttypes>
#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <stack>
#include <string>
#include <vector>

#include "ytlib/Common/shared_lib_def.h"

#if defined(log_EXPORTS)
  #define LOG_API YT_DECLSPEC_EXPORT
#else
  #define LOG_API YT_DECLSPEC_IMPORT
#endif

namespace ytlib {

enum {
  MAX_BUF_SIZE = 1000,
  MAX_LOG_SIZE = 1000,
};

enum class LOG_LEVEL : uint8_t {
  L_TRACE = 0,
  L_DEBUG,
  L_INFO,
  L_WARN,
  L_ERROR,
  L_FATAL,
  L_NONE,
};

using Ctx = std::shared_ptr<std::map<std::string, std::string> >;

struct LogData {
  LOG_LEVEL lvl;
  uint32_t thread_id;
  uint64_t time;
  Ctx ctx;
  char msg[MAX_BUF_SIZE];
};

using LogWriter = std::function<void(const LogData&)>;

class LOG_API Log {
 public:
  Log();
  ~Log();

  void SetLevel(LOG_LEVEL lvl) { lvl_ = lvl; }
  LOG_LEVEL Level() const { return lvl_; }

  void Trace(LOG_LEVEL lvl, const char* fmt, ...);
  void Trace(LOG_LEVEL lvl, const Ctx& ctx, const char* fmt, ...);

  void AddWriter(LogWriter func);

  void PushCtx(const Ctx& ctx);
  void PopCtx();

 private:
  LOG_LEVEL lvl_ = LOG_LEVEL::L_INFO;

  std::shared_mutex ctx_mutex_;
  std::stack<Ctx> ctx_stack_;

  std::vector<LogWriter> writers_;
  LogData log_buf[MAX_LOG_SIZE];
};
}  // namespace ytlib
