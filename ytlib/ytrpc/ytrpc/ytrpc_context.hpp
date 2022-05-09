#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <sstream>
#include <string>

#include "ytlib/misc/stl_util.hpp"

namespace ytlib {
namespace ytrpc {

/**
 * @brief rpc上下文
 * @note 提供3种能力：
 * 1、链式取消能力
 * 2、超时能力
 * 3、meta信息能力
 */
class Context {
 public:
  enum class DoneCode : uint32_t {
    UNKNOWN = 0,
    TIMEOUT,
    CANCEL,
    CALL_FAILED,
  };

  Context() {}
  ~Context() {}

  // done 相关功能，线程安全
  void Done(DoneCode code, const std::string& info = "") {
    if (std::atomic_exchange(&done_flag_, true)) return;
    code_ = code;
    done_info_ = info;
  }

  void DoTimeout(const std::string& info = "") {
    Done(DoneCode::TIMEOUT, info);
  }

  void DoCancel(const std::string& info = "") {
    Done(DoneCode::CANCEL, info);
  }

  void DoCallFailed(const std::string& info = "") {
    Done(DoneCode::CALL_FAILED, info);
  }

  bool IsDone() const { return done_flag_; }

  const DoneCode& Code() const { return code_; }

  const std::string& DoneInfo() const { return done_info_; }

  // timeout相关功能，线程不安全
  void SetDeadline(const std::chrono::system_clock::time_point& deadline) {
    deadline_ = deadline;
  }

  const std::chrono::system_clock::time_point& Deadline() const {
    return deadline_;
  }

  void SetTimeout(const std::chrono::system_clock::duration& timeout) {
    SetDeadline(std::chrono::system_clock::now() + timeout);
  }

  const std::chrono::system_clock::duration Timeout() const {
    return Deadline() - std::chrono::system_clock::now();
  }

  // meta信息功能，线程不安全
  std::map<std::string, std::string>& ContextKv() { return kv; }

  // debug功能，打印ctx
  std::string ToString() const {
    std::stringstream ss;
    ss << "is done: " << (done_flag_ ? "true" : "false")
       << ", code: " << static_cast<uint32_t>(code_)
       << ", done info: " << done_info_
       << ", timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(Timeout()).count() << "ms\n";
    ss << Map2Str(kv);
    return ss.str();
  }

 private:
  std::atomic_bool done_flag_ = false;
  DoneCode code_ = DoneCode::UNKNOWN;
  std::string done_info_;
  std::chrono::system_clock::time_point deadline_ = std::chrono::system_clock::time_point::max();
  std::map<std::string, std::string> kv;
};

}  // namespace ytrpc
}  // namespace ytlib
