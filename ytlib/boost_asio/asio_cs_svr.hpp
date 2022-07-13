/**
 * @file asio_cs_svr.hpp
 * @brief 基于boost.asio的cs架构服务端
 * @note 可用于游戏cs、聊天室等场景
 * @author WT
 * @date 2022-05-04
 */
#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

class AsioCsServer : public std::enable_shared_from_this<AsioCsServer> {
 public:
  using MsgHandleFunc = std::function<void(const boost::asio::ip::tcp::endpoint&, const std::shared_ptr<boost::asio::streambuf>&)>;

  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4(), 57634};  // 监听的地址
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

  /**
   * @brief cs连接架构服务端构造函数
   *
   * @param io_ptr
   * @param cfg
   */
  AsioCsServer(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioCsServer::Cfg& cfg)
      : cfg_(AsioCsServer::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioCsServer::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        acceptor_(mgr_strand_, cfg_.ep),
        acceptor_timer_(mgr_strand_),
        mgr_timer_(mgr_strand_),
        msg_handle_ptr_(std::make_shared<MsgHandleFunc>([](const boost::asio::ip::tcp::endpoint&, const std::shared_ptr<boost::asio::streambuf>&) {})) {}

  ~AsioCsServer() {}

  AsioCsServer(const AsioCsServer&) = delete;             ///< no copy
  AsioCsServer& operator=(const AsioCsServer&) = delete;  ///< no copy

  void SendMsg(const boost::asio::ip::tcp::endpoint& ep, const std::shared_ptr<boost::asio::streambuf>& msg_buf_ptr) {
    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self, ep, msg_buf_ptr]() {
          ASIO_DEBUG_HANDLE(cs_svr_send_msg_co);

          if (!run_flag_) [[unlikely]]
            return;

          auto finditr = session_ptr_map_.find(ep);
          if (finditr == session_ptr_map_.end()) {
            DBG_PRINT("cs svr can not find endpoint %s in session map", TcpEp2Str(ep).c_str());
            return;
          }

          auto& session_ptr = finditr->second;
          if (session_ptr && session_ptr->IsRunning()) {
            session_ptr->SendMsg(msg_buf_ptr);
          }
        });
  }

  void RegisterMsgHandleFunc(MsgHandleFunc&& handle) {
    msg_handle_ptr_ = std::make_shared<MsgHandleFunc>(std::move(handle));
  }

  void RegisterMsgHandleFunc(const MsgHandleFunc& handle) {
    msg_handle_ptr_ = std::make_shared<MsgHandleFunc>(handle);
  }

  /**
   * @brief 启动cs服务器
   *
   */
  void Start() {
    if (std::atomic_exchange(&start_flag_, true)) return;

    auto self = shared_from_this();
    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(cs_svr_acceptor_co);

          while (run_flag_) {
            try {
              // 如果链接数达到上限，则等待一段时间再试
              if (session_ptr_map_.size() >= cfg_.max_session_num) {
                acceptor_timer_.expires_after(cfg_.mgr_timer_dt);
                co_await acceptor_timer_.async_wait(boost::asio::use_awaitable);
                continue;
              }

              auto session_ptr = std::make_shared<AsioCsServer::Session>(io_ptr_, session_cfg_ptr_, msg_handle_ptr_);
              co_await acceptor_.async_accept(session_ptr->Socket(), boost::asio::use_awaitable);
              session_ptr->Start();

              session_ptr_map_.emplace(session_ptr->Socket().remote_endpoint(), session_ptr);

            } catch (const std::exception& e) {
              DBG_PRINT("cs svr accept connection get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);

    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(cs_svr_timer_co);

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
              DBG_PRINT("cs svr timer get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);
  }

  /**
   * @brief 停止cs服务器
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(cs_svr_stop_co);

          uint32_t stop_step = 1;
          while (stop_step) {
            try {
              switch (stop_step) {
                case 1:
                  acceptor_timer_.cancel();
                  ++stop_step;
                case 2:
                  mgr_timer_.cancel();
                  ++stop_step;
                case 3:
                  acceptor_.cancel();
                  ++stop_step;
                case 4:
                  acceptor_.close();
                  ++stop_step;
                case 5:
                  acceptor_.release();
                  ++stop_step;
                default:
                  stop_step = 0;
                  break;
              }
            } catch (const std::exception& e) {
              DBG_PRINT("cs svr stop get exception at step %u, exception info: %s", stop_step, e.what());
              ++stop_step;
            }
          }

          for (auto& session_ptr : session_ptr_map_)
            session_ptr.second->Stop();

          session_ptr_map_.clear();
        });
  }

 private:
  // 包头结构：| 2byte magicnum | 4byte msglen |
  static const size_t HEAD_SIZE = 6;
  static const char HEAD_BYTE_1 = 'Y';
  static const char HEAD_BYTE_2 = 'T';

  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : max_no_data_duration(cfg.max_no_data_duration),
          max_recv_size(cfg.max_recv_size) {}

    std::chrono::steady_clock::duration max_no_data_duration;
    uint32_t max_recv_size;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(const std::shared_ptr<boost::asio::io_context>& io_ptr,
            const std::shared_ptr<const AsioCsServer::SessionCfg>& session_cfg_ptr,
            const std::shared_ptr<MsgHandleFunc>& msg_handle_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          io_ptr_(io_ptr),
          session_socket_strand_(boost::asio::make_strand(*io_ptr)),
          sock_(session_socket_strand_),
          send_sig_timer_(session_socket_strand_),
          session_mgr_strand_(boost::asio::make_strand(*io_ptr)),
          timer_(session_mgr_strand_),
          msg_handle_ptr_(msg_handle_ptr) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void SendMsg(std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
      auto self = shared_from_this();
      boost::asio::dispatch(
          session_socket_strand_,
          [this, self, msg_buf_ptr]() {
            ASIO_DEBUG_HANDLE(cs_svr_session_send_msg_co);
            data_list.emplace_back(msg_buf_ptr);
            send_sig_timer_.cancel();
          });
    }

    void Start() {
      auto self = shared_from_this();

      // 发送协程
      boost::asio::co_spawn(
          session_socket_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(cs_svr_session_send_co);

            try {
              while (run_flag_) {
                while (!data_list.empty()) {
                  std::list<std::shared_ptr<boost::asio::streambuf> > tmp_data_list;
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

                  tick_has_data_ = true;
                  size_t write_data_size = co_await boost::asio::async_write(sock_, data_buf_vec, boost::asio::use_awaitable);
                  DBG_PRINT("cs svr session async write %llu bytes", write_data_size);
                }

                try {
                  send_sig_timer_.expires_at(std::chrono::steady_clock::time_point::max());
                  co_await send_sig_timer_.async_wait(boost::asio::use_awaitable);
                } catch (const std::exception& e) {
                  DBG_PRINT("cs svr session timer canceled, exception info: %s", e.what());
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("cs svr session send co get exception and exit, exception info: %s", e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 接收协程
      boost::asio::co_spawn(
          session_socket_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(cs_svr_session_recv_co);

            try {
              std::vector<char> head_buf(HEAD_SIZE);
              boost::asio::mutable_buffer asio_head_buf(head_buf.data(), HEAD_SIZE);
              while (run_flag_) {
                size_t read_data_size = co_await boost::asio::async_read(sock_, asio_head_buf, boost::asio::transfer_exactly(HEAD_SIZE), boost::asio::use_awaitable);
                DBG_PRINT("cs svr session async read %llu bytes for head", read_data_size);
                tick_has_data_ = true;

                if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
                  throw std::runtime_error("Get an invalid head.");

                uint32_t msg_len = GetUint32FromBuf(&head_buf[2]);

                if (msg_len > session_cfg_ptr_->max_recv_size) [[unlikely]]
                  throw std::runtime_error("Msg too large.");

                std::shared_ptr<boost::asio::streambuf> msg_buf = std::make_shared<boost::asio::streambuf>();

                read_data_size = co_await boost::asio::async_read(sock_, msg_buf->prepare(msg_len), boost::asio::transfer_exactly(msg_len), boost::asio::use_awaitable);
                DBG_PRINT("cs svr session async read %llu bytes", read_data_size);
                tick_has_data_ = true;

                msg_buf->commit(msg_len);
                boost::asio::post(*io_ptr_, std::bind(*msg_handle_ptr_, sock_.remote_endpoint(), msg_buf));
              }
            } catch (const std::exception& e) {
              DBG_PRINT("cs svr session recv co get exception and exit, exception info: %s", e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          session_mgr_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(cs_svr_session_timer_co);

            try {
              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->max_no_data_duration);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                } else {
                  DBG_PRINT("cs svr session exit due to timeout(%llums), addr %s.", std::chrono::duration_cast<std::chrono::milliseconds>(session_cfg_ptr_->max_no_data_duration).count(), TcpEp2Str(sock_.remote_endpoint()).c_str());
                  break;
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("cs svr session timer get exception and exit, addr %s, exception %s", TcpEp2Str(sock_.remote_endpoint()).c_str(), e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);
    }

    // 在stop的时候只是想办法结束各个协程，协程里面使用到的资源由各个协程自行释放
    void Stop() {
      if (!std::atomic_exchange(&run_flag_, false)) return;

      auto self = shared_from_this();
      boost::asio::dispatch(
          session_socket_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(cs_svr_session_sock_stop_co);

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
                DBG_PRINT("cs svr session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });

      boost::asio::dispatch(
          session_mgr_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(cs_svr_session_mgr_stop_co);

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
                DBG_PRINT("cs svr session mgr stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    boost::asio::ip::tcp::socket& Socket() { return sock_; }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const AsioCsServer::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    std::shared_ptr<boost::asio::io_context> io_ptr_;

    boost::asio::strand<boost::asio::io_context::executor_type> session_socket_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer send_sig_timer_;

    boost::asio::strand<boost::asio::io_context::executor_type> session_mgr_strand_;
    boost::asio::steady_timer timer_;

    std::atomic_bool tick_has_data_ = false;
    std::list<std::shared_ptr<boost::asio::streambuf> > data_list;

    std::shared_ptr<MsgHandleFunc> msg_handle_ptr_;
  };

 private:
  const AsioCsServer::Cfg cfg_;
  std::atomic_bool start_flag_ = false;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const AsioCsServer::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;                             // session池操作strand
  boost::asio::ip::tcp::acceptor acceptor_;                                                            // 监听器
  boost::asio::steady_timer acceptor_timer_;                                                           // 连接满时监听器的sleep定时器
  boost::asio::steady_timer mgr_timer_;                                                                // 管理session池的定时器
  std::map<boost::asio::ip::tcp::endpoint, std::shared_ptr<AsioCsServer::Session> > session_ptr_map_;  // session池

  std::shared_ptr<MsgHandleFunc> msg_handle_ptr_;
};

}  // namespace ytlib
