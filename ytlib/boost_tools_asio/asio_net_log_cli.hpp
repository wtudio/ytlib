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

#include <boost/asio.hpp>

#include "ytlib/boost_tools_asio/asio_debug_tools.hpp"
#include "ytlib/boost_tools_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief 远程日志服务器客户端
 * @note 必须以智能指针形式构造。内部有定时器，会定时一次性的发送一段时间内的日志到服务器
 */
class AsioNetLogClient : public std::enable_shared_from_this<AsioNetLogClient> {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint svr_ep;                                                // 服务端地址
    std::chrono::steady_clock::duration heart_beat_time = std::chrono::seconds(5);        // 定时器间隔
    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(60);  // 最长无数据时间

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.heart_beat_time < std::chrono::milliseconds(100)) cfg.heart_beat_time = std::chrono::milliseconds(100);

      return cfg;
    }
  };

  /**
   * @brief 远程日志服务器客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  AsioNetLogClient(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioNetLogClient::Cfg& cfg)
      : cfg_(AsioNetLogClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioNetLogClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioNetLogClient() = default;

  AsioNetLogClient(const AsioNetLogClient&) = delete;             ///< no copy
  AsioNetLogClient& operator=(const AsioNetLogClient&) = delete;  ///< no copy

  /**
   * @brief 打日志到远程日志服务器
   *
   * @param log_buf_ptr 日志内容
   */
  void LogToSvr(const std::shared_ptr<boost::asio::streambuf>& log_buf_ptr) {
    if (!run_flag_) [[unlikely]]
      return;

    std::shared_ptr<AsioNetLogClient::Session> cur_session_ptr;
    std::atomic_store(&cur_session_ptr, session_ptr_);
    if (cur_session_ptr && cur_session_ptr->IsRunning()) {
      cur_session_ptr->LogToSvr(log_buf_ptr);
      return;
    }

    // 当前session不能用，需要去新建session
    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self, log_buf_ptr]() {
          ASIO_DEBUG_HANDLE(net_log_cli_log_to_svr_co);

          if (!run_flag_) [[unlikely]]
            return;

          std::shared_ptr<AsioNetLogClient::Session> cur_session_ptr;
          std::atomic_store(&cur_session_ptr, session_ptr_);

          if (!cur_session_ptr || !cur_session_ptr->IsRunning()) {
            cur_session_ptr = std::make_shared<AsioNetLogClient::Session>(io_ptr_, session_cfg_ptr_);
            cur_session_ptr->Start();
            std::atomic_store(&session_ptr_, cur_session_ptr);
          }
          cur_session_ptr->LogToSvr(log_buf_ptr);
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

          std::shared_ptr<AsioNetLogClient::Session> cur_session_ptr;
          std::atomic_store(&cur_session_ptr, session_ptr_);

          if (cur_session_ptr) {
            cur_session_ptr->Stop();
            cur_session_ptr.reset();
          }
        });
  }

  /**
   * @brief 是否在运行
   *
   * @return const std::atomic_bool&
   */
  const std::atomic_bool& IsRunning() { return run_flag_; }

  /**
   * @brief 获取配置
   *
   * @return const AsioNetLogClient::Cfg&
   */
  const AsioNetLogClient::Cfg& GetCfg() const { return cfg_; }

 private:
  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : svr_ep(cfg.svr_ep),
          heart_beat_time(cfg.heart_beat_time),
          max_no_data_duration(cfg.max_no_data_duration) {}

    boost::asio::ip::tcp::endpoint svr_ep;
    std::chrono::steady_clock::duration heart_beat_time;
    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(const std::shared_ptr<boost::asio::io_context>& io_ptr,
            const std::shared_ptr<const AsioNetLogClient::SessionCfg>& session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(boost::asio::make_strand(*io_ptr)),
          sock_(session_strand_),
          timer_(session_strand_) {}

    ~Session() = default;

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void LogToSvr(const std::shared_ptr<boost::asio::streambuf>& log_buf_ptr) {
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

      // 定时发送协程
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
                timer_.expires_after(session_cfg_ptr_->heart_beat_time);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (!data_list.empty()) {
                  std::list<std::shared_ptr<boost::asio::streambuf> > tmp_data_list;
                  tmp_data_list.swap(data_list);

                  std::vector<boost::asio::const_buffer> data_buf_vec;
                  data_buf_vec.reserve(tmp_data_list.size());
                  for (auto& itr : tmp_data_list) {
                    data_buf_vec.emplace_back(itr->data());
                  }

                  size_t write_data_size = co_await boost::asio::async_write(sock_, data_buf_vec, boost::asio::use_awaitable);
                  DBG_PRINT("net log cli session async write %llu bytes", write_data_size);

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

            uint32_t stop_step = 1;
            while (stop_step) {
              try {
                switch (stop_step) {
                  case 1:
                    timer_.cancel();
                    ++stop_step;
                  case 2:
                    sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    ++stop_step;
                  case 3:
                    sock_.cancel();
                    ++stop_step;
                  case 4:
                    sock_.close();
                    ++stop_step;
                  case 5:
                    sock_.release();
                    ++stop_step;
                  default:
                    stop_step = 0;
                    break;
                }
              } catch (const std::exception& e) {
                DBG_PRINT("net log cli session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
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

 private:
  const AsioNetLogClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  std::shared_ptr<const AsioNetLogClient::SessionCfg> session_cfg_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::shared_ptr<AsioNetLogClient::Session> session_ptr_;
};

}  // namespace ytlib
