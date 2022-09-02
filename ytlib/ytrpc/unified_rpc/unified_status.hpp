#pragma once

#include <map>
#include <string>

namespace ytlib {
namespace ytrpc {

class UnifiedStatusConversionCenter {
 public:
  using UnifiedToNativeConversionFun = std::function<void(const UnifiedStatus&, std::shared_ptr<void>&)>;
  using NativeToUnifiedConversionFun = std::function<void(const std::shared_ptr<void>&, UnifiedStatus&)>;

  static UnifiedStatusConversionCenter& Ins() {
    static UnifiedStatusConversionCenter instance;
    return instance;
  }

  template <typename... Args>
  requires std::constructible_from<UnifiedToNativeConversionFun, Args...>
  void RegUnifiedToNativeConversionFun(const std::string& native_rpc_name, Args&&... args) {
    unified_to_native_fun_map_.emplace(native_rpc_name, (Args &&) args...);
  }

  template <typename... Args>
  requires std::constructible_from<NativeToUnifiedConversionFun, Args...>
  void RegNativeToUnifiedConversionFun(const std::string& native_rpc_name, Args&&... args) {
    native_to_unified_fun_map_.emplace(native_rpc_name, (Args &&) args...);
  }

  UnifiedToNativeConversionFun GetUnifiedToNativeConversionFun(const std::string& native_rpc_name) const {
    auto finditr = unified_to_native_fun_map_.find(native_rpc_name);
    if (finditr != unified_to_native_fun_map_.end()) {
      return finditr->second;
    }
    return UnifiedToNativeConversionFun();
  }

  NativeToUnifiedConversionFun GetNativeToUnifiedConversionFun(const std::string& native_rpc_name) const {
    auto finditr = native_to_unified_fun_map_.find(native_rpc_name);
    if (finditr != native_to_unified_fun_map_.end()) {
      return finditr->second;
    }
    return NativeToUnifiedConversionFun();
  }

 private:
  UnifiedStatusConversionCenter() {}

  std::map<std::string, UnifiedToNativeConversionFun> unified_to_native_fun_map_;
  std::map<std::string, NativeToUnifiedConversionFun> native_to_unified_fun_map_;
};

class UnifiedStatus {
 public:
  explicit UnifiedStatus(bool ok = true, const std::string& msg = "")
      : ok_(ok), msg_(msg) {}

  ~UnifiedStatus() {}

  operator bool() const { return ok_; }
  bool OK() const { return suok_c_; }
  void SetOK(bool ok) { ok_ = ok; }

  const std::string& Msg() const { return msg_; }
  void SetMsg(const std::string& msg) { msg_ = msg; }

  // 原生status转换相关功能
  std::shared_ptr<void> GetNativeStatus(const std::string& native_rpc_name) {
    auto finditr = native_status_map_.find(native_rpc_name);
    if (finditr != native_status_map_.end()) {
      return finditr->second;
    }

    auto fun = UnifiedStatusConversionCenter::Ins().GetUnifiedToNativeConversionFun(native_rpc_name);
    if (!fun) {
      return std::shared_ptr<void>();
    }

    std::shared_ptr<void> native_status;
    fun(*this, native_status);
    native_status_map_.emplace(native_rpc_name, native_status);

    return native_status;
  }

 private:
  bool ok_ = true;   // 是否成功
  std::string msg_;  // 业务错误信息

  struct NativeStruct {
    bool mut = false;           // 是否需要根据原生ptr更新UnifiedStatus
    std::shared_ptr<void> ptr;  // 原生结构
  };
  std::map<std::string, NativeStruct> native_status_map_;
};

}  // namespace ytrpc
}  // namespace ytlib