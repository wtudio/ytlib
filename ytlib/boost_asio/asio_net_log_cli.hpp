/**
 * @file asio_net_log_cli.hpp
 * @brief 基于boost.asio的远程日志客户端
 * @note 基于boost.asio的远程日志客户端
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <atomic>
#include <memory>

#include <boost/asio.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief 远程日志服务器客户端
 * @note 必须以智能指针形式构造，在结束使用前手动调用Stop方法
 */
class NetLogClient : public std::enable_shared_from_this<NetLogClient> {
 public:
  /**
   * @brief 远程日志服务器客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param log_svr_ep 日志服务器地址
   */
  NetLogClient(std::shared_ptr<boost::asio::io_context> io_ptr, const boost::asio::ip::tcp::endpoint& log_svr_ep)
      : log_svr_ep_(log_svr_ep),
        io_ptr_(io_ptr),
        strand_(boost::asio::make_strand(*io_ptr_)),
        sock_(strand_) {}

  ~NetLogClient() {}

  NetLogClient(const NetLogClient&) = delete;             ///< no copy
  NetLogClient& operator=(const NetLogClient&) = delete;  ///< no copy

  /**
   * @brief 停止
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (std::atomic_exchange(&stop_flag_, true)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        strand_,
        [self]() {
          ASIO_DEBUG_HANDLE(net_log_cli_stop_co);
          self->CloseSocket();
        });
  }

  /**
   * @brief 打日志到远程日志服务器
   *
   * @param log_buf_ptr 日志内容指针
   */
  void LogToSvr(std::shared_ptr<boost::asio::streambuf> log_buf_ptr) {
    auto self = shared_from_this();
    boost::asio::co_spawn(
        strand_,
        [self, log_buf_ptr]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(net_log_cli_log_to_svr_co);
          co_await self->LogToSvrCo(log_buf_ptr);
          co_return;
        },
        boost::asio::detached);
  }

  /**
   * @brief 打日志到远程日志服务器协程
   * @note 调用者需要保证再协程执行完之前自身不析构
   * @param log_buf_ptr 日志内容指针
   * @return boost::asio::awaitable<void> 协程句柄
   */
  boost::asio::awaitable<void> LogToSvrCo(std::shared_ptr<boost::asio::streambuf> log_buf_ptr) {
    if (stop_flag_) [[unlikely]]
      co_return;

    uint32_t retry_num = max_retry_num_;

    do {
      try {
        if (!sock_.is_open()) {
          DBG_PRINT("start create a new connect to %s", TcpEp2Str(log_svr_ep_).c_str());
          co_await sock_.async_connect(log_svr_ep_, boost::asio::use_awaitable);
        }

        while (log_buf_ptr->size()) {
          size_t n = co_await sock_.async_write_some(log_buf_ptr->data(), boost::asio::use_awaitable);
          log_buf_ptr->consume(n);
        }

        co_return;

      } catch (const std::exception& e) {
        DBG_PRINT("send log to svr get exception, addr %s, exception %s", TcpEp2Str(log_svr_ep_).c_str(), e.what());
        CloseSocket();
      }
    } while (--retry_num);

    DBG_PRINT("log to svr failed after %u retry.", max_retry_num_);

    co_return;
  }

 private:
  void CloseSocket() {
    if (sock_.is_open()) {
      sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
      sock_.cancel();
      sock_.close();
      sock_.release();
    }
  }

 private:
  const uint32_t max_retry_num_ = 5;
  const boost::asio::ip::tcp::endpoint log_svr_ep_;
  std::atomic_bool stop_flag_ = false;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::asio::ip::tcp::socket sock_;
};

}  // namespace ytlib
