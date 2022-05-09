#pragma once

#include <sstream>
#include <string>

namespace ytlib {
namespace ytrpc {

enum class StatusCode : int32_t {
  OK = 0,
  UNKNOWN,
  TIMEOUT,
  CANCELLED,
  CTX_DONE,

  // svr side
  NOT_IMPLEMENTED,
  NOT_FOUND,
  SVR_PARSE_REQ_FAILED,
  SVR_SERIALIZE_RSP_FAILED,

  // cli side
  CLI_SERIALIZE_REQ_FAILED,
  CLI_PARSE_RSP_FAILED,
  CLI_IS_NOT_RUNNING,

  MAX_NUM,
};

class Status {
 public:
  Status() {}

  Status(int func_ret, const std::string& func_ret_msg)
      : ret_(StatusCode::OK),
        func_ret_(func_ret),
        func_ret_msg_(func_ret_msg) {}

  Status(StatusCode ret, int func_ret = 0, const std::string& func_ret_msg = "")
      : ret_(ret),
        func_ret_(func_ret),
        func_ret_msg_(func_ret_msg) {}

  ~Status() {}

  operator bool() const {
    return ret_ == StatusCode::OK && func_ret_ == 0;
  }

  const StatusCode& Ret() const { return ret_; }
  const int32_t& FuncRet() const { return func_ret_; }
  const std::string& FuncRetMsg() const { return func_ret_msg_; }

  std::string ToString() const {
    std::stringstream ss;
    ss << "ret: " << static_cast<int32_t>(ret_) << ", func ret: " << func_ret_ << " func ret msg: " << func_ret_msg_;
    return ss.str();
  }

 private:
  StatusCode ret_ = StatusCode::OK;  // 框架层面错误码
  int32_t func_ret_ = 0;             // 业务错误
  std::string func_ret_msg_;         // 业务错误信息
};

}  // namespace ytrpc
}  // namespace ytlib
