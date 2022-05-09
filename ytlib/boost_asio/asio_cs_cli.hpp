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

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief cs连接架构客户端
 * @note 必须以智能指针形式构造。会通过定时器进行连接保活。单连接发送接收，不保证发送实时性，极端情况可能要等待较长时间
 */
class AsioCsClient : public std::enable_shared_from_this<AsioCsClient> {
 public:
  using MsgHandleFunc = std::function<void(std::shared_ptr<boost::asio::streambuf>)>;

  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint svr_ep;  // 服务端地址

    std::chrono::steady_clock::duration timer_dt = std::chrono::seconds(60);  // 定时器间隔

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.timer_dt < std::chrono::milliseconds(100)) cfg.timer_dt = std::chrono::milliseconds(100);

      return cfg;
    }
  };

  /**
   * @brief cs连接架构客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  AsioCsClient(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioCsClient::Cfg& cfg)
      : cfg_(AsioCsClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioCsClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        msg_handle_ptr_(std::make_shared<MsgHandleFunc>([](std::shared_ptr<boost::asio::streambuf>) {})) {}

  ~AsioCsClient() {}

  AsioCsClient(const AsioCsClient&) = delete;             ///< no copy
  AsioCsClient& operator=(const AsioCsClient&) = delete;  ///< no copy

  /**
   * @brief 发送消息到服务端
   *
   * @param msg_buf_ptr 消息内容
   */
  void SendMsg(std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self, msg_buf_ptr]() {
          ASIO_DEBUG_HANDLE(cs_cli_send_msg_to_svr_co);

          if (!run_flag_) [[unlikely]]
            return;

          if (!session_ptr_ || !session_ptr_->IsRunning()) {
            session_ptr_ = std::make_shared<AsioCsClient::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_, io_ptr_, msg_handle_ptr_);
            session_ptr_->Start();
          }
          session_ptr_->SendMsg(msg_buf_ptr);
        });
  }

  void RegisterMsgHandleFunc(MsgHandleFunc&& handle) {
    msg_handle_ptr_ = std::make_shared<MsgHandleFunc>(std::move(handle));
  }

  void RegisterMsgHandleFunc(const MsgHandleFunc& handle) {
    msg_handle_ptr_ = std::make_shared<MsgHandleFunc>(handle);
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
          if (session_ptr_) {
            session_ptr_->Stop();
            session_ptr_.reset();
          }
        });
  }

 private:
  // 包头结构：| 2byte magicnum | 4byte msglen |
  static const size_t HEAD_SIZE = 6;
  static const char HEAD_BYTE_1 = 'Y';
  static const char HEAD_BYTE_2 = 'T';

  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : svr_ep(cfg.svr_ep),
          timer_dt(cfg.timer_dt) {}

    boost::asio::ip::tcp::endpoint svr_ep;
    std::chrono::steady_clock::duration timer_dt;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(boost::asio::strand<boost::asio::io_context::executor_type> session_strand,
            std::shared_ptr<const AsioCsClient::SessionCfg> session_cfg_ptr,
            std::shared_ptr<boost::asio::io_context> io_ptr,
            std::shared_ptr<MsgHandleFunc> msg_handle_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          sock_(session_strand_),
          sig_timer_(session_strand_),
          io_ptr_(io_ptr),
          msg_handle_ptr_(msg_handle_ptr) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void SendMsg(std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
      auto self = shared_from_this();
      boost::asio::dispatch(
          session_strand_,
          [this, self, msg_buf_ptr]() {
            ASIO_DEBUG_HANDLE(cs_cli_session_send_msg_to_svr_co);
            data_list.emplace_back(msg_buf_ptr);
            sig_timer_.cancel();
          });
    }

    void Start() {
      auto self = shared_from_this();

      // 启动协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(cs_cli_session_start_co);

            try {
              DBG_PRINT("cs cli session start create a new connect to %s", TcpEp2Str(session_cfg_ptr_->svr_ep).c_str());
              co_await sock_.async_connect(session_cfg_ptr_->svr_ep, boost::asio::use_awaitable);

              // 发送协程
              boost::asio::co_spawn(
                  session_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(cs_cli_session_send_co);

                    try {
                      while (run_flag_) {
                        while (!data_list.empty()) {
                          std::list<std::shared_ptr<boost::asio::streambuf> > tmp_data_list;
                          tmp_data_list.swap(data_list);

                          std::vector<char> head_buf(tmp_data_list.size() * HEAD_SIZE);

                          std::list<boost::asio::const_buffer> data_buf_list;
                          size_t ct = 0;
                          for (auto& itr : tmp_data_list) {
                            head_buf[ct * HEAD_SIZE] = HEAD_BYTE_1;
                            head_buf[ct * HEAD_SIZE + 1] = HEAD_BYTE_2;
                            SetBufFromUint32(&head_buf[ct * HEAD_SIZE + 2], static_cast<uint32_t>(itr->size()));
                            data_buf_list.emplace_back(boost::asio::const_buffer(&head_buf[ct * HEAD_SIZE], HEAD_SIZE));
                            ++ct;

                            data_buf_list.emplace_back(itr->data());
                          }

                          size_t write_data_size = co_await boost::asio::async_write(sock_, data_buf_list, boost::asio::use_awaitable);
                          DBG_PRINT("cs cli session async write %llu bytes", write_data_size);
                        }

                        bool heartbeat_flag = false;
                        try {
                          sig_timer_.expires_after(session_cfg_ptr_->timer_dt);
                          co_await sig_timer_.async_wait(boost::asio::use_awaitable);
                          heartbeat_flag = true;
                        } catch (const std::exception& e) {
                          DBG_PRINT("cs cli session timer canceled, exception info: %s", e.what());
                        }

                        if (heartbeat_flag) {
                          // 心跳包仅用来保活，不传输业务/管理信息
                          const static char heartbeat_pkg[HEAD_SIZE] = {HEAD_BYTE_1, HEAD_BYTE_2, 0, 0, 0, 0};

                          size_t write_data_size = co_await boost::asio::async_write(sock_, boost::asio::const_buffer(heartbeat_pkg, HEAD_SIZE), boost::asio::use_awaitable);
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
                  session_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(cs_cli_session_recv_co);

                    try {
                      std::vector<char> head_buf(HEAD_SIZE);
                      while (run_flag_) {
                        size_t read_data_size = co_await boost::asio::async_read(sock_, boost::asio::buffer(head_buf, HEAD_SIZE), boost::asio::transfer_exactly(HEAD_SIZE), boost::asio::use_awaitable);
                        DBG_PRINT("cs cli session async read %llu bytes for head", read_data_size);

                        if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
                          throw std::runtime_error("Get an invalid head.");

                        uint32_t msg_len = GetUint32FromBuf(&head_buf[2]);
                        if (msg_len == 0) [[unlikely]]
                          continue;

                        std::shared_ptr<boost::asio::streambuf> msg_buf = std::make_shared<boost::asio::streambuf>();

                        read_data_size = co_await boost::asio::async_read(sock_, msg_buf->prepare(msg_len), boost::asio::transfer_exactly(msg_len), boost::asio::use_awaitable);
                        DBG_PRINT("cs cli session async read %llu bytes", read_data_size);

                        if (read_data_size != msg_len) [[unlikely]]
                          throw std::runtime_error("Get an invalid pkg.");

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
          session_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(cs_cli_session_stop_co);

            uint32_t stop_step = 1;
            while (stop_step) {
              try {
                switch (stop_step) {
                  case 1:
                    sig_timer_.cancel();
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
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer sig_timer_;

    std::list<std::shared_ptr<boost::asio::streambuf> > data_list;
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

}  // namespace ytlib
