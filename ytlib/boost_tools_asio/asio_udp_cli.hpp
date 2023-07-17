
/**
 * @file asio_udp_cli.hpp
 * @brief 基于boost.asio的udp客户端
 * @note 可用于网络环境较好、允许丢包的场景
 * @author WT
 * @date 2023-06-26
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

class AsioUdpClient : public std::enable_shared_from_this<AsioUdpClient> {
 public:
  struct Cfg {
    boost::asio::ip::udp::endpoint svr_ep;                                                // 服务端地址
    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(60);  // 最长无数据时间
    size_t max_package_size = 1024;                                                       // 每包最大长度。不可大于65507

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.max_package_size > 65507) cfg.max_package_size = 65507;

      return cfg;
    }
  };

  AsioUdpClient(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioUdpClient::Cfg& cfg)
      : cfg_(AsioUdpClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioUdpClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioUdpClient() = default;

  AsioUdpClient(const AsioUdpClient&) = delete;             ///< no copy
  AsioUdpClient& operator=(const AsioUdpClient&) = delete;  ///< no copy

  /**
   * @brief 发送消息到服务端
   *
   * @param msg_buf_ptr 消息内容
   */
  void SendMsg(const std::shared_ptr<boost::asio::streambuf>& msg_buf_ptr) {
    if (!run_flag_) [[unlikely]]
      return;

    if (msg_buf_ptr->size() > cfg_.max_package_size) [[unlikely]]
      throw std::runtime_error("Msg too large for udp package.");

    std::shared_ptr<AsioUdpClient::Session> cur_session_ptr;
    std::atomic_store(&cur_session_ptr, session_ptr_);
    if (cur_session_ptr && cur_session_ptr->IsRunning()) {
      cur_session_ptr->SendMsg(msg_buf_ptr);
      return;
    }

    // 当前session不能用，需要去新建session
    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self, msg_buf_ptr]() {
          ASIO_DEBUG_HANDLE(udp_cli_send_msg_co);

          if (!run_flag_) [[unlikely]]
            return;

          std::shared_ptr<AsioUdpClient::Session> cur_session_ptr;
          std::atomic_store(&cur_session_ptr, session_ptr_);

          if (!cur_session_ptr || !cur_session_ptr->IsRunning()) {
            cur_session_ptr = std::make_shared<AsioUdpClient::Session>(io_ptr_, session_cfg_ptr_);
            cur_session_ptr->Start();
            std::atomic_store(&session_ptr_, cur_session_ptr);
          }

          cur_session_ptr->SendMsg(msg_buf_ptr);
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
          ASIO_DEBUG_HANDLE(udp_cli_stop_co);

          std::shared_ptr<AsioUdpClient::Session> cur_session_ptr;
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
   * @return const AsioUdpClient::Cfg&
   */
  const AsioUdpClient::Cfg& GetCfg() const { return cfg_; }

 private:
  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : svr_ep(cfg.svr_ep),
          max_no_data_duration(cfg.max_no_data_duration) {}

    boost::asio::ip::udp::endpoint svr_ep;
    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(const std::shared_ptr<boost::asio::io_context>& io_ptr,
            const std::shared_ptr<const AsioUdpClient::SessionCfg>& session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_socket_strand_(boost::asio::make_strand(*io_ptr)),
          sock_(session_socket_strand_),
          session_mgr_strand_(boost::asio::make_strand(*io_ptr)),
          timer_(session_mgr_strand_) {
      sock_.open(boost::asio::ip::udp::v4());
    }

    ~Session() = default;

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void SendMsg(const std::shared_ptr<boost::asio::streambuf>& msg_buf_ptr) {
      auto self = shared_from_this();
      boost::asio::co_spawn(
          session_socket_strand_,
          [this, self, msg_buf_ptr]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(udp_cli_session_send_msg_co);
            try {
              size_t data_size = msg_buf_ptr->size();
              size_t send_data_size = co_await sock_.async_send_to(msg_buf_ptr->data(), session_cfg_ptr_->svr_ep, boost::asio::use_awaitable);
              if (send_data_size != data_size) {
                DBG_PRINT("warning: udp send msg incomplete, expected send data size %llu, actual send data size %llu", data_size, send_data_size);
              }

              tick_has_data_ = true;
            } catch (const std::exception& e) {
              DBG_PRINT("udp send msg get exception, exception info: %s", e.what());
            }
            co_return;
          },
          boost::asio::detached);
    }

    void Start() {
      auto self = shared_from_this();

      // 启动协程
      boost::asio::co_spawn(
          session_mgr_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(udp_cli_session_timer_co);

            try {
              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->max_no_data_duration);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                } else {
                  DBG_PRINT("udp cli session exit due to timeout(%llums).",
                            std::chrono::duration_cast<std::chrono::milliseconds>(session_cfg_ptr_->max_no_data_duration).count());
                  break;
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("udp cli session timer get exception and exit, exception info: %s", e.what());
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
          session_socket_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(udp_cli_session_stop_co);

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
                  default:
                    stop_step = 0;
                    break;
                }
              } catch (const std::exception& e) {
                DBG_PRINT("udp cli session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });

      boost::asio::dispatch(
          session_mgr_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(udp_cli_session_mgr_stop_co);

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
                DBG_PRINT("udp cli session mgr stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const AsioUdpClient::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;

    boost::asio::strand<boost::asio::io_context::executor_type> session_socket_strand_;
    boost::asio::ip::udp::socket sock_;

    boost::asio::strand<boost::asio::io_context::executor_type> session_mgr_strand_;
    boost::asio::steady_timer timer_;

    std::atomic_bool tick_has_data_ = false;
  };

 private:
  const AsioUdpClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  std::shared_ptr<const AsioUdpClient::SessionCfg> session_cfg_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::shared_ptr<AsioUdpClient::Session> session_ptr_;
};

class AsioUdpClientPool : public std::enable_shared_from_this<AsioUdpClientPool> {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    size_t max_client_num = 1000;  // 最大client数

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.max_client_num < 10) cfg.max_client_num = 10;

      return cfg;
    }
  };

  /**
   * @brief net client pool构造函数
   *
   * @param io_ptr io_context
   * @param cfg 配置
   */
  AsioUdpClientPool(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioUdpClientPool::Cfg& cfg)
      : cfg_(AsioUdpClientPool::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioUdpClientPool() = default;

  AsioUdpClientPool(const AsioUdpClientPool&) = delete;             ///< no copy
  AsioUdpClientPool& operator=(const AsioUdpClientPool&) = delete;  ///< no copy

  /**
   * @brief 获取net client
   * @note 如果net client目的地址相同，则会复用已有的net client
   * @param cfg net client的配置
   * @return asio::awaitable<std::shared_ptr<AsioUdpClient> > net client
   */
  boost::asio::awaitable<std::shared_ptr<AsioUdpClient>> GetClient(const AsioUdpClient::Cfg& cfg) {
    return boost::asio::co_spawn(
        mgr_strand_,
        [this, &cfg]() -> boost::asio::awaitable<std::shared_ptr<AsioUdpClient>> {
          if (!run_flag_) [[unlikely]]
            throw std::runtime_error("Net client is closed.");

          const size_t client_hash = std::hash<boost::asio::ip::udp::endpoint>{}(cfg.svr_ep);

          auto itr = client_map_.find(client_hash);
          if (itr != client_map_.end()) {
            if (itr->second->IsRunning()) co_return itr->second;
            client_map_.erase(itr);
          }

          if (client_map_.size() >= cfg_.max_client_num) [[unlikely]] {
            for (auto itr = client_map_.begin(); itr != client_map_.end();) {
              if (itr->second->IsRunning())
                ++itr;
              else
                client_map_.erase(itr++);
            }

            if (client_map_.size() >= cfg_.max_client_num) [[unlikely]]
              throw std::runtime_error("Net client num reach the upper limit.");
          }

          std::shared_ptr<AsioUdpClient> client_ptr = std::make_shared<AsioUdpClient>(io_ptr_, cfg);
          client_map_.emplace(client_hash, client_ptr);
          co_return client_ptr;
        },
        boost::asio::use_awaitable);
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
          for (auto& itr : client_map_)
            itr.second->Stop();

          client_map_.clear();
        });
  }

  /**
   * @brief 是否在运行
   *
   * @return const std::atomic_bool&
   */
  const std::atomic_bool& IsRunning() { return run_flag_; }

 private:
  const AsioUdpClientPool::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::map<size_t, std::shared_ptr<AsioUdpClient>> client_map_;
};

}  // namespace ytlib
