#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <string>

#include "ytlib/misc/stl_util.hpp"

namespace ytlib {
namespace ytrpc {

class UnifexRpcContextInf {
 public:
  UnifexRpcContextInf() {}
  ~UnifexRpcContextInf() {}

  // done 相关功能，线程安全
  void Done(const std::string& info = "") const {
    if (std::atomic_exchange(&done_flag_, true)) return;
    done_info_ = info;
  }

  bool IsDone() const { return done_flag_; }

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
  std::map<std::string, std::string>& ContextKv() { return kv_; }

  const std::map<std::string, std::string>& ContextKv() const { return kv_; }

  // debug功能，打印ctx
  std::string ToString() const {
    std::stringstream ss;
    ss << "is done: " << (done_flag_ ? "true" : "false")
       << ", done info: " << done_info_
       << ", timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(Timeout()).count() << "ms\n";
    ss << Map2Str(kv_);
    return ss.str();
  }

  // 原生context转换相关功能
  void RegInfToNativeFun(const std::string& native_rpc_name,
                         std::function<void(const UnifexRpcContextInf&, std::shared_ptr<void>&)>&& inf_to_native_fun) {
    inf_to_native_fun_map_.emplace(native_rpc_name, std::move(inf_to_native_fun));
  }

  void RegNativeToInfFun(const std::string& native_rpc_name,
                         std::function<void(const std::shared_ptr<void>&, UnifexRpcContextInf&)>&& native_to_inf_fun) {
    native_to_inf_fun_map_.emplace(native_rpc_name, std::move(native_to_inf_fun));
  }

  std::shared_ptr<void> GetNativeCtxHandle(const std::string& native_rpc_name) {
    auto finditr = native_ctx_handle_map_.find(native_rpc_name);
    if (finditr != native_ctx_handle_map_.end()) {
      return finditr->second;
    }

    auto finditr2 = inf_to_native_fun_map_.find(native_rpc_name);
    if (finditr2 == inf_to_native_fun_map_.end()) {
      return std::shared_ptr<void>();
    }

    std::shared_ptr<void> result;
    finditr2->second(*this, result);
    native_ctx_handle_map_.emplace(native_rpc_name, result);

    return result;
  }

 private:
  mutable std::atomic_bool done_flag_ = false;
  mutable std::string done_info_;

  std::chrono::system_clock::time_point deadline_ = std::chrono::system_clock::time_point::max();

  std::map<std::string, std::string> kv_;

  std::map<std::string, std::shared_ptr<void>> native_ctx_handle_map_;

  std::map<std::string, std::function<void(const UnifexRpcContextInf&, std::shared_ptr<void>&)>> inf_to_native_fun_map_;
  std::map<std::string, std::function<void(const std::shared_ptr<void>&, UnifexRpcContextInf&)>> native_to_inf_fun_map_;
};

}  // namespace ytrpc
}  // namespace ytlib