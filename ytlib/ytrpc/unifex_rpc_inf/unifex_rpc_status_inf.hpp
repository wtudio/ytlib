#pragma once

namespace ytlib {
namespace ytrpc {

class UnifexRpcStatusInf {
 public:
  // 提取一些通用的框架层面错误码
  enum class Code : int32_t {
    OK = 0,     // 正常
    UNKNOWN,    // 未知
    TIMEOUT,    // 超时
    CANCELLED,  // 因为ctx done而取消调用

    // svr side
    NOT_IMPLEMENTED,       // 服务未实现
    NOT_FOUND,             // 服务未找到
    SVR_PARSE_REQ_FAILED,  // 服务端解析req包出错

    // cli side
    CLI_PARSE_RSP_FAILED,  // 客户端解析rsp包出错
    CLI_IS_NOT_RUNNING,    // 客户端已关闭

    MAX_NUM,
  };

 public:
  UnifexRpcStatusInf() {}

  UnifexRpcStatusInf(int func_ret, const std::string& func_ret_msg)
      : ret_(UnifexRpcStatusInf::Code::OK),
        func_ret_(func_ret),
        func_ret_msg_(func_ret_msg) {}

  UnifexRpcStatusInf(UnifexRpcStatusInf::Code ret, int func_ret = 0, const std::string& func_ret_msg = "")
      : ret_(ret),
        func_ret_(func_ret),
        func_ret_msg_(func_ret_msg) {}

  ~UnifexRpcStatusInf() {}

  operator bool() const {
    return ret_ == UnifexRpcStatusInf::Code::OK && func_ret_ == 0;
  }

  const UnifexRpcStatusInf::Code& Ret() const { return ret_; }
  const int32_t& FuncRet() const { return func_ret_; }
  const std::string& FuncRetMsg() const { return func_ret_msg_; }

  std::string ToString() const {
    std::stringstream ss;
    ss << "ret: " << static_cast<int32_t>(ret_) << ", func ret: " << func_ret_ << " func ret msg: " << func_ret_msg_;
    return ss.str();
  }

  std::shared_ptr<void> GetNativeStatusHandle(const std::string& native_rpc_name) {
    return std::shared_ptr<void>();
  }

 private:
  UnifexRpcStatusInf::Code ret_ = UnifexRpcStatusInf::Code::OK;  // 框架层面错误码
  int32_t func_ret_ = 0;                                         // 业务错误
  std::string func_ret_msg_;                                     // 业务错误信息

  std::map<std::string, std::shared_ptr<void> > native_status_handles_map_;
};

}  // namespace ytrpc
}  // namespace ytlib