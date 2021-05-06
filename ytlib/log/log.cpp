#include "log.hpp"

#include <mutex>
#include "console_writer.hpp"
#include "rotate_file_writer.hpp"

namespace ytlib {

Log::Log() {
  writers_.emplace_back(GetConsoleWriter(std::map<std::string, std::string>()));
  writers_.emplace_back(GetRotateFileWriter(std::map<std::string, std::string>()));
}

Log::~Log() {
}

void Log::Trace(LOG_LEVEL lvl, const char* fmt, ...) {
  ctx_mutex_.lock_shared();
  Ctx ctx = ctx_stack_.empty() ? Ctx() : ctx_stack_.top();
  ctx_mutex_.unlock_shared();
}

void Log::Trace(LOG_LEVEL lvl, const Ctx& ctx, const char* fmt, ...) {
}

void Log::AddWriter(LogWriter func) {
}

void Log::PushCtx(const Ctx& ctx) {
  std::unique_lock<std::shared_mutex> lck(ctx_mutex_);
  ctx_stack_.emplace(ctx);
}
void Log::PopCtx() {
  std::unique_lock<std::shared_mutex> lck(ctx_mutex_);
  if (!ctx_stack_.empty())
    ctx_stack_.pop();
}
}  // namespace ytlib
