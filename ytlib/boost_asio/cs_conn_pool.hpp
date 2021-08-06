/**
 * @file cs_conn_pool.hpp
 * @brief cs类型连接池
 * @note 基于boost.asio的cs类型连接池
 * @author WT
 * @date 2021-07-26
 */
#pragma once

#include "net_util.hpp"

#include <functional>
#include <map>

namespace ytlib {

/**
 * @brief tcp客户端连接池配置
 */
struct CliConnPoolCfg {
  CliConnPoolCfg() {}
};

/**
 * @brief tcp客户端连接池类（todo）
 * 提供Send、Recv、SendAndRecv方法
 */
class CliConnPool : public std::enable_shared_from_this<CliConnPool> {
 public:
  CliConnPool(boost::asio::io_context& io, const CliConnPoolCfg& cfg) : io_(io),
                                                                        mgr_strand_(boost::asio::make_strand(io_)),
                                                                        cfg_(cfg) {}
  ~CliConnPool() {}

  CliConnPool(const CliConnPool&) = delete;             ///<no copy
  CliConnPool& operator=(const CliConnPool&) = delete;  ///<no copy

  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;
  }

 private:
  // 提供Send、Recv、SendAndRecv方法，缓存tcp连接
  class CliConn : public std::enable_shared_from_this<CliConn> {
   public:
    CliConn(boost::asio::io_context& io, const TcpEp& ep) : ep_(ep),
                                                            strand_(boost::asio::make_strand(io)),
                                                            sock_(strand_) {}
    ~CliConn() {}

    CliConn(const CliConn&) = delete;             ///<no copy
    CliConn& operator=(const CliConn&) = delete;  ///<no copy

    void Stop() {
      if (!std::atomic_exchange(&run_flag_, false)) return;
    }

   private:
    const TcpEp ep_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    TcpSocket sock_;
    std::atomic_bool run_flag_ = true;
  };

 private:
  boost::asio::io_context& io_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;  // 连接池操作strand
  std::map<TcpEp, std::shared_ptr<CliConn> > conn_map_;                     // 连接池
  std::atomic_bool run_flag_ = true;
  CliConnPoolCfg cfg_;
};

/**
 * @brief tcp服务端连接池类（todo）
 * 提供Send、Recv、SendAndRecv方法
 */
class SvrConnPool {
};

}  // namespace ytlib
