/**
 * @file asio_net_log_cli.hpp
 * @brief 基于boost.asio的远程日志客户端
 * @note 基于boost.asio的远程日志客户端
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <atomic>
#include <chrono>
#include <list>
#include <memory>
#include <queue>

#include <boost/asio.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief 远程日志服务器客户端
 * @note 必须以智能指针形式构造
 */
class AsioNetLogClient : public std::enable_shared_from_this<AsioNetLogClient> {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint svr_ep;  // 服务端地址

    std::chrono::steady_clock::duration timer_dt = std::chrono::seconds(5);               // 定时器间隔
    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(60);  // 最长无数据时间

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.timer_dt < std::chrono::seconds(1)) cfg.timer_dt = std::chrono::seconds(1);
      if (cfg.max_no_data_duration < cfg.timer_dt * 2) cfg.max_no_data_duration = cfg.timer_dt * 2;

      return cfg;
    }
  };

  /**
   * @brief 远程日志服务器客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  AsioNetLogClient(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioNetLogClient::Cfg& cfg)
      : cfg_(AsioNetLogClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioNetLogClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioNetLogClient() {}

  AsioNetLogClient(const AsioNetLogClient&) = delete;             ///< no copy
  AsioNetLogClient& operator=(const AsioNetLogClient&) = delete;  ///< no copy

  /**
   * @brief 打日志到远程日志服务器
   *
   * @param log_buf_ptr 日志内容
   */
  void LogToSvr(std::shared_ptr<boost::asio::streambuf> log_buf_ptr) {
    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self, log_buf_ptr]() {
          ASIO_DEBUG_HANDLE(net_log_cli_log_to_svr_co);

          if (!run_flag_) [[unlikely]]
            return;

          if (!session_ptr_ || !session_ptr_->IsRunning()) {
            session_ptr_ = std::make_shared<AsioNetLogClient::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_);
            session_ptr_->Start();
          }
          session_ptr_->LogToSvr(log_buf_ptr);
        });
  }

  /**
   * @brief 停止
   *
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(net_log_cli_stop_co);
          if (session_ptr_) {
            session_ptr_->Stop();
            session_ptr_.reset();
          }
        });
  }

 private:
  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : svr_ep(cfg.svr_ep),
          timer_dt(cfg.timer_dt),
          max_no_data_duration(cfg.max_no_data_duration) {}

    boost::asio::ip::tcp::endpoint svr_ep;
    std::chrono::steady_clock::duration timer_dt;
    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(boost::asio::strand<boost::asio::io_context::executor_type> session_strand,
            std::shared_ptr<const AsioNetLogClient::SessionCfg> session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          sock_(session_strand_),
          timer_(session_strand_) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void LogToSvr(std::shared_ptr<boost::asio::streambuf> log_buf_ptr) {
      auto self = shared_from_this();
      boost::asio::dispatch(
          session_strand_,
          [this, self, log_buf_ptr]() {
            ASIO_DEBUG_HANDLE(net_log_cli_session_log_to_svr_co);
            data_list.emplace_back(log_buf_ptr);
          });
    }

    void Start() {
      auto self = shared_from_this();
      // 定时器协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(net_log_cli_session_timer_co);

            try {
              namespace chrono = std::chrono;

              DBG_PRINT("net log cli session start create a new connect to %s", TcpEp2Str(session_cfg_ptr_->svr_ep).c_str());
              co_await sock_.async_connect(session_cfg_ptr_->svr_ep, boost::asio::use_awaitable);

              chrono::steady_clock::time_point last_data_time_point = chrono::steady_clock::now();  // 上次有数据时间

              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->timer_dt);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (!data_list.empty()) {
                  std::list<boost::asio::streambuf::const_buffers_type> data_buf_list;
                  std::queue<size_t> data_buf_size_queue;
                  size_t total_data_size = 0;

                  for (auto& itr : data_list) {
                    data_buf_list.emplace_back(itr->data());
                    total_data_size += data_buf_size_queue.emplace(itr->size());
                  }

                  while (!data_list.empty()) {
                    DBG_PRINT("net log cli session start async write %llu bytes to %s", total_data_size, TcpEp2Str(session_cfg_ptr_->svr_ep).c_str());
                    size_t write_data_size = co_await sock_.async_write_some(data_buf_list, boost::asio::use_awaitable);
                    DBG_PRINT("net log cli session async write %llu bytes", write_data_size);

                    if (write_data_size >= total_data_size) {
                      data_list.clear();
                    } else {
                      total_data_size -= write_data_size;
                      while (write_data_size >= data_buf_size_queue.front()) {
                        write_data_size -= data_buf_size_queue.front();
                        data_list.pop_front();
                        data_buf_list.pop_front();
                        data_buf_size_queue.pop();
                      }
                      data_list.front()->consume(write_data_size);
                    }
                  }

                  last_data_time_point = chrono::steady_clock::now();
                } else {
                  chrono::steady_clock::duration no_data_duration = chrono::steady_clock::now() - last_data_time_point;
                  if (no_data_duration >= session_cfg_ptr_->max_no_data_duration) {
                    DBG_PRINT("net log cli session exit due to timeout(%llums).", chrono::duration_cast<chrono::milliseconds>(no_data_duration).count());
                    break;
                  }
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("net log cli session timer get exception and exit, exception info: %s", e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);
    }

    void Stop() {
      if (!std::atomic_exchange(&run_flag_, false)) return;

      auto self = shared_from_this();
      boost::asio::dispatch(
          session_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(net_log_cli_session_stop_co);

            try {
              timer_.cancel();
            } catch (const std::exception& e) {
              DBG_PRINT("net log cli session timer cancel get exception, exception info: %s", e.what());
            }

            try {
              sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            } catch (const std::exception& e) {
              DBG_PRINT("net log cli session socket shutdown get exception, exception info: %s", e.what());
            }

            try {
              sock_.cancel();
            } catch (const std::exception& e) {
              DBG_PRINT("net log cli session socket cancel get exception, exception info: %s", e.what());
            }

            try {
              sock_.close();
            } catch (const std::exception& e) {
              DBG_PRINT("net log cli session socket close get exception, exception info: %s", e.what());
            }

            try {
              sock_.release();
            } catch (const std::exception& e) {
              DBG_PRINT("net log cli session socket release get exception, exception info: %s", e.what());
            }
          });
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const AsioNetLogClient::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer timer_;

    std::list<std::shared_ptr<boost::asio::streambuf> > data_list;
  };

  const AsioNetLogClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const AsioNetLogClient::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::shared_ptr<AsioNetLogClient::Session> session_ptr_;
};

}  // namespace ytlib
