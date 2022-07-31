#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include <boost/asio.hpp>

#include <unifex/task.hpp>

#include "ytlib/execution/execution_tools.hpp"

#include "unifex_rpc_context.hpp"
#include "unifex_rpc_status.hpp"

namespace ytlib {
namespace ytrpc {

/**
 * @brief 基于libunifex作为并发接口形式的rpc实现
 * 底层网络仍然使用asio
 */
class UnifexRpcClient : public std::enable_shared_from_this<UnifexRpcClient> {
 public:
  struct Cfg {
    boost::asio::ip::tcp::endpoint svr_ep;                                           // 服务端地址
    std::chrono::steady_clock::duration heart_beat_time = std::chrono::seconds(60);  // 心跳包间隔
    uint32_t max_recv_size = 1024 * 1024 * 10;                                       // 回包最大尺寸，最大10m

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.heart_beat_time < std::chrono::milliseconds(100)) cfg.heart_beat_time = std::chrono::milliseconds(100);

      return cfg;
    }
  };

  explicit UnifexRpcClient(const UnifexRpcClient::Cfg& cfg)
      : cfg_(UnifexRpcClient::Cfg::Verify(cfg)) {}

  ~UnifexRpcClient() {}

  UnifexRpcClient(const UnifexRpcClient&) = delete;             ///< no copy
  UnifexRpcClient& operator=(const UnifexRpcClient&) = delete;  ///< no copy

  const UnifexRpcClient::Cfg& GetCfg() const { return cfg_; }

 private:
  const UnifexRpcClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
};

class UnifexRpcServiceProxy {
 protected:
  explicit UnifexRpcServiceProxy(const std::shared_ptr<UnifexRpcClient>& client_ptr)
      : client_ptr_(client_ptr) {}

  virtual ~UnifexRpcServiceProxy() {}

 private:
  std::shared_ptr<UnifexRpcClient> client_ptr_;
};

}  // namespace ytrpc
}  // namespace ytlib