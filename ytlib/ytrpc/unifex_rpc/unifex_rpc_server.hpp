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

class UnifexRpcService {
 protected:
  UnifexRpcService() {}
  virtual ~UnifexRpcService() {}

 private:
};

class UnifexRpcServer : public std::enable_shared_from_this<UnifexRpcServer> {
 public:
  struct Cfg {
    boost::asio::ip::tcp::endpoint ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4(), 51965};  // 监听的地址
    size_t max_session_num = 1000000;                                                                          // 最大连接数
    std::chrono::steady_clock::duration mgr_timer_dt = std::chrono::seconds(10);                               // 管理协程定时器间隔
    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(300);                      // 最长无数据时间
    uint32_t max_recv_size = 1024 * 1024 * 10;                                                                 // 包最大尺寸，最大10m

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.max_session_num < 1) cfg.max_session_num = 1;
      if (cfg.max_session_num > boost::asio::ip::tcp::acceptor::max_listen_connections)
        cfg.max_session_num = boost::asio::ip::tcp::acceptor::max_listen_connections;
      if (cfg.mgr_timer_dt < std::chrono::milliseconds(100)) cfg.mgr_timer_dt = std::chrono::milliseconds(100);

      if (cfg.max_no_data_duration < std::chrono::seconds(10)) cfg.max_no_data_duration = std::chrono::seconds(10);

      return cfg;
    }
  };

  UnifexRpcServer(const UnifexRpcServer::Cfg& cfg)
      : cfg_(UnifexRpcServer::Cfg::Verify(cfg)) {}

  ~UnifexRpcServer() {}

  UnifexRpcServer(const UnifexRpcServer&) = delete;             ///< no copy
  UnifexRpcServer& operator=(const UnifexRpcServer&) = delete;  ///< no copy

  template <std::derived_from<UnifexRpcService> ServiceType>
  void RegisterService(const std::shared_ptr<ServiceType>& service_ptr) {
  }

  const UnifexRpcServer::Cfg& GetCfg() const { return cfg_; }

 private:
  const UnifexRpcServer::Cfg cfg_;
  std::atomic_bool start_flag_ = false;
  std::atomic_bool run_flag_ = true;
};

}  // namespace ytrpc
}  // namespace ytlib