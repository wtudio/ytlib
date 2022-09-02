#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <string>
#include <unordered_map>

namespace ytlib {
namespace ytrpc {

class UnifiedContext {
 public:
  explicit UnifiedContext() {}
  ~UnifiedContext() {}

  // done 相关功能，线程安全
  void Done(const std::string& info = "") const {
    if (std::atomic_exchange(&done_flag_, true)) return;
    done_info_ = info;
  }

  bool IsDone() const { return done_flag_; }

  const std::string& DoneInfo() const { return done_info_; }

  // timeout相关功能，线程不安全
  void SetDeadline(const std::chrono::system_clock::time_point& deadline) { deadline_ = deadline; }
  const std::chrono::system_clock::time_point& Deadline() const { return deadline_; }
  void SetTimeout(const std::chrono::system_clock::duration& timeout) { SetDeadline(std::chrono::system_clock::now() + timeout); }
  const std::chrono::system_clock::duration Timeout() const { return Deadline() - std::chrono::system_clock::now(); }

  // meta信息功能，线程不安全。todo：优化转换拷贝开销，尝试使用string_view？
  std::unordered_map<std::string, std::string>& Meta() { return meta_; }
  const std::unordered_map<std::string, std::string>& Meta() const { return meta_; }
  void AddMeta(const std::string& k, const std::string& v) { meta_.emplace(k, v); }

  // debug功能，打印ctx
  std::string ToString() const {
    std::stringstream ss;
    ss << (done_flag_ ? "ctx is done" : "ctx is not done")
       << ", done info: " << done_info_
       << ", timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(Timeout()).count() << "ms"
       << ", meta size: " << meta_.size()
       << ".\n";

    for (const auto& kv : meta_) {
      ss << kv.first << ":" << kv.second << "\n";
    }

    return ss.str();
  }

  // 原生context转换相关功能
  void RegInfToNativeFun(const std::string& native_rpc_name,
                         std::function<void(const UnifiedContext&, std::shared_ptr<void>&)>&& inf_to_native_fun) {
    inf_to_native_fun_map_.emplace(native_rpc_name, std::move(inf_to_native_fun));
  }

  void RegNativeToInfFun(const std::string& native_rpc_name,
                         std::function<void(const std::shared_ptr<void>&, UnifiedContext&)>&& native_to_inf_fun) {
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
  std::unordered_map<std::string, std::string> meta_;

  std::map<std::string, std::shared_ptr<void>> native_ctx_handle_map_;
  std::map<std::string, std::function<void(const UnifiedContext&, std::shared_ptr<void>&)>> inf_to_native_fun_map_;
  std::map<std::string, std::function<void(const std::shared_ptr<void>&, UnifiedContext&)>> native_to_inf_fun_map_;
};

}  // namespace ytrpc
}  // namespace ytlib