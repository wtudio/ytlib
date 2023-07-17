/**
 * @file asio_cs_cli.hpp
 * @brief 基于boost.asio的cs架构客户端
 * @note 可用于游戏cs、聊天室等场景
 * @author WT
 * @date 2022-05-04
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
 * @brief cs连接架构客户端
 * @note 必须以智能指针形式构造。会通过定时器进行连接保活。单连接发送接收，不保证发送实时性，极端情况可能要等待较长时间
 */
class AsioCsClient : public std::enable_shared_from_this<AsioCsClient> {
 public:
  using MsgHandleFunc = std::function<void(const std::shared_ptr<boost::asio::streambuf>&)>;

  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint svr_ep;                                           // 服务端地址
    std::chrono::steady_clock::duration heart_beat_time = std::chrono::seconds(60);  // 定时器间隔
    uint32_t max_recv_size = 1024 * 1024 * 10;                                       // 包最大尺寸，最大10m

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.heart_beat_time < std::chrono::milliseconds(100)) cfg.heart_beat_time = std::chrono::milliseconds(100);

      return cfg;
    }
  };

  /**
   * @brief cs连接架构客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  AsioCsClient(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioCsClient::Cfg& cfg)
      : cfg_(AsioCsClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioCsClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        msg_handle_ptr_(std::make_shared<MsgHandleFunc>([](const std::shared_ptr<boost::asio::streambuf>&) {})) {}

  ~AsioCsClient() = default;

  AsioCsClient(const AsioCsClient&) = delete;             ///< no copy
  AsioCsClient& operator=(const AsioCsClient&) = delete;  ///< no copy

  /**
   * @brief 发送消息到服务端
   *
   * @param msg_buf_ptr 消息内容
   */
  void SendMsg(const std::shared_ptr<boost::asio::streambuf>& msg_buf_ptr) {
    if (!run_flag_) [[unlikely]]
      return;

    std::shared_ptr<AsioCsClient::Session> cur_session_ptr;
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
          ASIO_DEBUG_HANDLE(cs_cli_send_msg_co);

          if (!run_flag_) [[unlikely]]
            return;

          std::shared_ptr<AsioCsClient::Session> cur_session_ptr;
          std::atomic_store(&cur_session_ptr, session_ptr_);

          if (!cur_session_ptr || !cur_session_ptr->IsRunning()) {
            cur_session_ptr = std::make_shared<AsioCsClient::Session>(io_ptr_, session_cfg_ptr_, msg_handle_ptr_);
            cur_session_ptr->Start();
            std::atomic_store(&session_ptr_, cur_session_ptr);
          }

          cur_session_ptr->SendMsg(msg_buf_ptr);
        });
  }

  template <typename... Args>
    requires std::constructible_from<MsgHandleFunc, Args...>
  void RegisterMsgHandleFunc(Args&&... args) {
    msg_handle_ptr_ = std::make_shared<MsgHandleFunc>(std::forward<Args>(args)...);
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
          ASIO_DEBUG_HANDLE(cs_cli_stop_co);

          std::shared_ptr<AsioCsClient::Session> cur_session_ptr;
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
   * @return const AsioCsClient::Cfg&
   */
  const AsioCsClient::Cfg& GetCfg() const { return cfg_; }

 private:
  // 包头结构：| 2byte magic num | 4byte msg len |
  static constexpr size_t HEAD_SIZE = 6;
  static constexpr char HEAD_BYTE_1 = 'Y';
  static constexpr char HEAD_BYTE_2 = 'T';

  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : svr_ep(cfg.svr_ep),
          heart_beat_time(cfg.heart_beat_time),
          max_recv_size(cfg.max_recv_size) {}

    boost::asio::ip::tcp::endpoint svr_ep;
    std::chrono::steady_clock::duration heart_beat_time;
    uint32_t max_recv_size;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(const std::shared_ptr<boost::asio::io_context>& io_ptr,
            const std::shared_ptr<const AsioCsClient::SessionCfg>& session_cfg_ptr,
            const std::shared_ptr<MsgHandleFunc>& msg_handle_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_socket_strand_(boost::asio::make_strand(*io_ptr)),
          sock_(session_socket_strand_),
          send_sig_timer_(session_socket_strand_),
          io_ptr_(io_ptr),
          msg_handle_ptr_(msg_handle_ptr) {}

    ~Session() = default;

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void SendMsg(const std::shared_ptr<boost::asio::streambuf>& msg_buf_ptr) {
      auto self = shared_from_this();
      boost::asio::dispatch(
          session_socket_strand_,
          [this, self, msg_buf_ptr]() {
            ASIO_DEBUG_HANDLE(cs_cli_session_send_msg_co);
            data_list.emplace_back(msg_buf_ptr);
            send_sig_timer_.cancel();
          });
    }

    void Start() {
      auto self = shared_from_this();

      // 启动协程
      boost::asio::co_spawn(
          session_socket_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(cs_cli_session_start_co);

            try {
              DBG_PRINT("cs cli session start create a new connect to %s", TcpEp2Str(session_cfg_ptr_->svr_ep).c_str());
              co_await sock_.async_connect(session_cfg_ptr_->svr_ep, boost::asio::use_awaitable);

              // 发送协程
              boost::asio::co_spawn(
                  session_socket_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(cs_cli_session_send_co);

                    try {
                      while (run_flag_) {
                        while (!data_list.empty()) {
                          std::list<std::shared_ptr<boost::asio::streambuf>> tmp_data_list;
                          tmp_data_list.swap(data_list);

                          std::vector<char> head_buf(tmp_data_list.size() * HEAD_SIZE);

                          std::vector<boost::asio::const_buffer> data_buf_vec;
                          data_buf_vec.reserve(tmp_data_list.size() * 2);
                          size_t ct = 0;
                          for (auto& itr : tmp_data_list) {
                            head_buf[ct * HEAD_SIZE] = HEAD_BYTE_1;
                            head_buf[ct * HEAD_SIZE + 1] = HEAD_BYTE_2;
                            SetBufFromUint32(&head_buf[ct * HEAD_SIZE + 2], static_cast<uint32_t>(itr->size()));
                            data_buf_vec.emplace_back(boost::asio::const_buffer(&head_buf[ct * HEAD_SIZE], HEAD_SIZE));
                            ++ct;

                            data_buf_vec.emplace_back(itr->data());
                          }

                          size_t write_data_size = co_await boost::asio::async_write(sock_, data_buf_vec, boost::asio::use_awaitable);
                          DBG_PRINT("cs cli session async write %llu bytes", write_data_size);
                        }

                        bool heartbeat_flag = false;
                        try {
                          send_sig_timer_.expires_after(session_cfg_ptr_->heart_beat_time);
                          co_await send_sig_timer_.async_wait(boost::asio::use_awaitable);
                          heartbeat_flag = true;
                        } catch (const std::exception& e) {
                          DBG_PRINT("cs cli session timer canceled, exception info: %s", e.what());
                        }

                        if (heartbeat_flag) {
                          // 心跳包仅用来保活，不传输业务/管理信息
                          const static char heartbeat_pkg[HEAD_SIZE] = {HEAD_BYTE_1, HEAD_BYTE_2, 0, 0, 0, 0};
                          const static boost::asio::const_buffer heartbeat_buf(heartbeat_pkg, HEAD_SIZE);

                          size_t write_data_size = co_await boost::asio::async_write(sock_, heartbeat_buf, boost::asio::use_awaitable);
                          DBG_PRINT("cs cli session async write %llu bytes for heartbeat", write_data_size);
                        }
                      }
                    } catch (const std::exception& e) {
                      DBG_PRINT("cs cli session send co get exception and exit, exception info: %s", e.what());
                    }

                    Stop();

                    co_return;
                  },
                  boost::asio::detached);

              // 接收协程
              boost::asio::co_spawn(
                  session_socket_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(cs_cli_session_recv_co);

                    try {
                      std::vector<char> head_buf(HEAD_SIZE);
                      boost::asio::mutable_buffer asio_head_buf(head_buf.data(), HEAD_SIZE);
                      while (run_flag_) {
                        size_t read_data_size = co_await boost::asio::async_read(sock_, asio_head_buf, boost::asio::transfer_exactly(HEAD_SIZE), boost::asio::use_awaitable);
                        DBG_PRINT("cs cli session async read %llu bytes for head", read_data_size);

                        if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
                          throw std::runtime_error("Get an invalid head.");

                        uint32_t msg_len = GetUint32FromBuf(&head_buf[2]);

                        if (msg_len > session_cfg_ptr_->max_recv_size) [[unlikely]]
                          throw std::runtime_error("Msg too large.");

                        std::shared_ptr<boost::asio::streambuf> msg_buf = std::make_shared<boost::asio::streambuf>();

                        read_data_size = co_await boost::asio::async_read(sock_, msg_buf->prepare(msg_len), boost::asio::transfer_exactly(msg_len), boost::asio::use_awaitable);
                        DBG_PRINT("cs cli session async read %llu bytes", read_data_size);

                        msg_buf->commit(msg_len);
                        boost::asio::post(*io_ptr_, std::bind(*msg_handle_ptr_, msg_buf));
                      }
                    } catch (const std::exception& e) {
                      DBG_PRINT("cs cli session recv co get exception and exit, exception info: %s", e.what());
                    }

                    Stop();

                    co_return;
                  },
                  boost::asio::detached);

            } catch (const std::exception& e) {
              DBG_PRINT("cs cli session start co get exception and exit, exception info: %s", e.what());
              Stop();
            }

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
            ASIO_DEBUG_HANDLE(cs_cli_session_stop_co);

            uint32_t stop_step = 1;
            while (stop_step) {
              try {
                switch (stop_step) {
                  case 1:
                    send_sig_timer_.cancel();
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
                DBG_PRINT("cs cli session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const AsioCsClient::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;

    boost::asio::strand<boost::asio::io_context::executor_type> session_socket_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer send_sig_timer_;
    std::list<std::shared_ptr<boost::asio::streambuf>> data_list;

    std::shared_ptr<boost::asio::io_context> io_ptr_;
    std::shared_ptr<MsgHandleFunc> msg_handle_ptr_;
  };

 private:
  const AsioCsClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  std::shared_ptr<const AsioCsClient::SessionCfg> session_cfg_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::shared_ptr<AsioCsClient::Session> session_ptr_;

  std::shared_ptr<MsgHandleFunc> msg_handle_ptr_;
};

class AsioCsClientPool : public std::enable_shared_from_this<AsioCsClientPool> {
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
  AsioCsClientPool(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioCsClientPool::Cfg& cfg)
      : cfg_(AsioCsClientPool::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioCsClientPool() = default;

  AsioCsClientPool(const AsioCsClientPool&) = delete;             ///< no copy
  AsioCsClientPool& operator=(const AsioCsClientPool&) = delete;  ///< no copy

  /**
   * @brief 获取net client
   * @note 如果net client目的地址相同，则会复用已有的net client
   * @param cfg net client的配置
   * @return asio::awaitable<std::shared_ptr<AsioCsClient> > net client
   */
  boost::asio::awaitable<std::shared_ptr<AsioCsClient>> GetClient(const AsioCsClient::Cfg& cfg) {
    return boost::asio::co_spawn(
        mgr_strand_,
        [this, &cfg]() -> boost::asio::awaitable<std::shared_ptr<AsioCsClient>> {
          if (!run_flag_) [[unlikely]]
            throw std::runtime_error("Net client is closed.");

          const size_t client_hash = std::hash<boost::asio::ip::tcp::endpoint>{}(cfg.svr_ep);

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

          std::shared_ptr<AsioCsClient> client_ptr = std::make_shared<AsioCsClient>(io_ptr_, cfg);
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
  const AsioCsClientPool::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::map<size_t, std::shared_ptr<AsioCsClient>> client_map_;
};

}  // namespace ytlib
