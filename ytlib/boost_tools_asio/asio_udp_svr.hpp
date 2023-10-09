/**
 * @file asio_udp_svr.hpp
 * @brief 基于boost.asio的udp服务端
 * @note 可用于网络环境较好、允许丢包的场景
 * @author WT
 * @date 2023-06-26
 */
#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "ytlib/boost_tools_asio/asio_debug_tools.hpp"
#include "ytlib/boost_tools_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {
class AsioUdpServer : public std::enable_shared_from_this<AsioUdpServer> {
 public:
  using MsgHandleFunc = std::function<void(const boost::asio::ip::udp::endpoint&, const std::shared_ptr<boost::asio::streambuf>&)>;

  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::udp::endpoint ep = boost::asio::ip::udp::endpoint{boost::asio::ip::address_v4(), 53927};  // 监听的地址
    size_t max_session_num = 1000000;                                                                          // 最大连接数
    std::chrono::steady_clock::duration mgr_timer_dt = std::chrono::seconds(10);                               // 管理协程定时器间隔
    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(300);                      // 最长无数据时间
    size_t max_package_size = 1024;                                                                            // 每包最大长度。不可大于65507

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.max_session_num < 1) cfg.max_session_num = 1;
      if (cfg.mgr_timer_dt < std::chrono::milliseconds(100)) cfg.mgr_timer_dt = std::chrono::milliseconds(100);
      if (cfg.max_no_data_duration < std::chrono::seconds(10)) cfg.max_no_data_duration = std::chrono::seconds(10);
      if (cfg.max_package_size > 65507) cfg.max_package_size = 65507;

      return cfg;
    }
  };

  AsioUdpServer(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioUdpServer::Cfg& cfg)
      : cfg_(AsioUdpServer::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        socket_strand_(boost::asio::make_strand(*io_ptr_)),
        sock_(socket_strand_, cfg_.ep),
        session_cfg_ptr_(std::make_shared<const AsioUdpServer::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        acceptor_timer_(mgr_strand_),
        mgr_timer_(mgr_strand_),
        msg_handle_ptr_(std::make_shared<MsgHandleFunc>([](const boost::asio::ip::udp::endpoint&, const std::shared_ptr<boost::asio::streambuf>&) {})) {}

  ~AsioUdpServer() = default;

  AsioUdpServer(const AsioUdpServer&) = delete;
  AsioUdpServer& operator=(const AsioUdpServer&) = delete;

  template <typename... Args>
    requires std::constructible_from<MsgHandleFunc, Args...>
  void RegisterMsgHandleFunc(Args&&... args) {
    msg_handle_ptr_ = std::make_shared<MsgHandleFunc>(std::forward<Args>(args)...);
  }

  /**
   * @brief 启动udp服务器
   *
   */
  void Start() {
    if (std::atomic_exchange(&start_flag_, true)) return;

    auto self = shared_from_this();
    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(udp_svr_recv_co);

          while (run_flag_) {
            try {
              // 如果链接数达到上限，则等待一段时间再试
              if (session_ptr_map_.size() >= cfg_.max_session_num) {
                acceptor_timer_.expires_after(cfg_.mgr_timer_dt);
                co_await acceptor_timer_.async_wait(boost::asio::use_awaitable);
                continue;
              }

              std::shared_ptr<boost::asio::streambuf> msg_buf = std::make_shared<boost::asio::streambuf>();
              boost::asio::ip::udp::endpoint remote_ep;
              size_t read_data_size = co_await sock_.async_receive_from(msg_buf->prepare(cfg_.max_package_size), remote_ep, boost::asio::use_awaitable);
              msg_buf->commit(read_data_size);

              std::shared_ptr<AsioUdpServer::Session> session_ptr;

              auto finditr = session_ptr_map_.find(remote_ep);
              if (finditr != session_ptr_map_.end()) {
                session_ptr = finditr->second;
              } else {
                session_ptr = std::make_shared<AsioUdpServer::Session>(io_ptr_, session_cfg_ptr_, remote_ep, msg_handle_ptr_);
                session_ptr->Start();
                session_ptr_map_.emplace(remote_ep, session_ptr);
              }

              session_ptr->HandleMsg(std::move(msg_buf));

            } catch (const std::exception& e) {
              DBG_PRINT("udp svr accept connection get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);

    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(udp_svr_timer_co);

          while (run_flag_) {
            try {
              mgr_timer_.expires_after(cfg_.mgr_timer_dt);
              co_await mgr_timer_.async_wait(boost::asio::use_awaitable);

              for (auto itr = session_ptr_map_.begin(); itr != session_ptr_map_.end();) {
                if (itr->second->IsRunning())
                  ++itr;
                else
                  session_ptr_map_.erase(itr++);
              }
            } catch (const std::exception& e) {
              DBG_PRINT("udp svr timer get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);
  }

  /**
   * @brief 停止udp服务器
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(udp_svr_stop_co);

          uint32_t stop_step = 1;
          while (stop_step) {
            try {
              switch (stop_step) {
                case 1:
                  sock_.shutdown(boost::asio::ip::udp::socket::shutdown_both);
                  ++stop_step;
                case 2:
                  sock_.cancel();
                  ++stop_step;
                case 3:
                  sock_.close();
                  ++stop_step;
                case 4:
                  sock_.release();
                  ++stop_step;
                case 5:
                  acceptor_timer_.cancel();
                  ++stop_step;
                case 6:
                  mgr_timer_.cancel();
                  ++stop_step;
                default:
                  stop_step = 0;
                  break;
              }
            } catch (const std::exception& e) {
              DBG_PRINT("udp svr stop get exception at step %u, exception info: %s", stop_step, e.what());
              ++stop_step;
            }
          }

          for (auto& session_ptr : session_ptr_map_)
            session_ptr.second->Stop();

          session_ptr_map_.clear();
        });
  }

  /**
   * @brief 获取配置
   *
   * @return const AsioUdpServer::Cfg&
   */
  const AsioUdpServer::Cfg& GetCfg() const { return cfg_; }

 private:
  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : max_no_data_duration(cfg.max_no_data_duration) {}

    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(const std::shared_ptr<boost::asio::io_context>& io_ptr,
            const std::shared_ptr<const AsioUdpServer::SessionCfg>& session_cfg_ptr,
            const boost::asio::ip::udp::endpoint& remote_ep,
            const std::shared_ptr<MsgHandleFunc>& msg_handle_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          io_ptr_(io_ptr),
          remote_ep_(remote_ep),
          session_mgr_strand_(boost::asio::make_strand(*io_ptr)),
          timer_(session_mgr_strand_),
          msg_handle_ptr_(msg_handle_ptr) {}

    ~Session() = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    void HandleMsg(std::shared_ptr<boost::asio::streambuf>&& msg_buf_ptr) {
      auto self = shared_from_this();
      boost::asio::dispatch(
          *io_ptr_,
          [this, self, msg_buf_ptr{std::move(msg_buf_ptr)}]() {
            ASIO_DEBUG_HANDLE(udp_svr_session_handle_co);

            tick_has_data_ = true;

            (*msg_handle_ptr_)(remote_ep_, msg_buf_ptr);
          });
    }

    void Start() {
      auto self = shared_from_this();

      // 定时器协程
      boost::asio::co_spawn(
          session_mgr_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(udp_svr_session_timer_co);

            try {
              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->max_no_data_duration);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                } else {
                  DBG_PRINT("udp svr session exit due to timeout(%llums), addr %s.",
                            std::chrono::duration_cast<std::chrono::milliseconds>(session_cfg_ptr_->max_no_data_duration).count(),
                            UdpEp2Str(remote_ep_).c_str());
                  break;
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("udp svr session timer get exception and exit, addr %s, exception %s", UdpEp2Str(remote_ep_).c_str(), e.what());
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
          session_mgr_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(udp_svr_session_mgr_stop_co);

            uint32_t stop_step = 1;
            while (stop_step) {
              try {
                switch (stop_step) {
                  case 1:
                    timer_.cancel();
                    ++stop_step;
                  default:
                    stop_step = 0;
                    break;
                }
              } catch (const std::exception& e) {
                DBG_PRINT("udp svr session mgr stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const AsioUdpServer::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    std::shared_ptr<boost::asio::io_context> io_ptr_;

    const boost::asio::ip::udp::endpoint remote_ep_;

    boost::asio::strand<boost::asio::io_context::executor_type> session_mgr_strand_;
    boost::asio::steady_timer timer_;

    std::atomic_bool tick_has_data_ = false;

    std::shared_ptr<MsgHandleFunc> msg_handle_ptr_;
  };

 private:
  const AsioUdpServer::Cfg cfg_;
  std::atomic_bool start_flag_ = false;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> socket_strand_;
  boost::asio::ip::udp::socket sock_;

  std::shared_ptr<const AsioUdpServer::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;                              // session池操作strand
  boost::asio::steady_timer acceptor_timer_;                                                            // 连接满时监听器的sleep定时器
  boost::asio::steady_timer mgr_timer_;                                                                 // 管理session池的定时器
  std::map<boost::asio::ip::udp::endpoint, std::shared_ptr<AsioUdpServer::Session> > session_ptr_map_;  // session池

  std::shared_ptr<MsgHandleFunc> msg_handle_ptr_;
};
}  // namespace ytlib
