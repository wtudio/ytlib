#pragma once

#include <concepts>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

#include "ytrpc_buffer.hpp"
#include "ytrpc_context.hpp"
#include "ytrpc_status.hpp"

#include "Head.pb.h"

namespace ytlib {
namespace ytrpc {

class RpcServer : public std::enable_shared_from_this<RpcServer> {
  friend class RpcService;

 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    uint16_t port = 55399;  // 监听的端口

    size_t max_session_num = 1000000;                                             // 最大连接数
    std::chrono::steady_clock::duration mgr_timer_dt = std::chrono::seconds(10);  // 管理协程定时器间隔

    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(300);  // 最长无数据时间

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.port > 65535 || cfg.port < 1000) cfg.port = 55399;

      if (cfg.max_session_num < 1) cfg.max_session_num = 1;
      if (cfg.max_session_num > boost::asio::ip::tcp::acceptor::max_listen_connections)
        cfg.max_session_num = boost::asio::ip::tcp::acceptor::max_listen_connections;
      if (cfg.mgr_timer_dt < std::chrono::milliseconds(100)) cfg.mgr_timer_dt = std::chrono::milliseconds(100);

      if (cfg.max_no_data_duration < std::chrono::seconds(10)) cfg.max_no_data_duration = std::chrono::seconds(10);

      return cfg;
    }
  };

  /**
   * @brief rpc server构造函数
   *
   * @param io_ptr
   * @param cfg
   */
  RpcServer(std::shared_ptr<boost::asio::io_context> io_ptr, const RpcServer::Cfg& cfg)
      : cfg_(RpcServer::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const RpcServer::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        acceptor_(mgr_strand_, boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4(), cfg_.port}),
        acceptor_timer_(mgr_strand_),
        mgr_timer_(mgr_strand_),
        func_map_ptr_(std::make_shared<std::unordered_map<std::string, AdapterFunc>>()) {}

  ~RpcServer() {}

  RpcServer(const RpcServer&) = delete;             ///< no copy
  RpcServer& operator=(const RpcServer&) = delete;  ///< no copy

  template <std::derived_from<RpcService> ServiceType>
  void RegisterService(std::shared_ptr<ServiceType> service_ptr) {
    std::shared_ptr<RpcService> rpc_service_ptr = std::static_pointer_cast<RpcService>(service_ptr);
    service_ptr_list_.emplace_back(rpc_service_ptr);
    func_map_ptr_->insert(rpc_service_ptr->FuncMap().begin(), rpc_service_ptr->FuncMap().end());

    if (start_flag_)
      throw std::runtime_error("Should not register service after server start.");
  }

  /**
   * @brief 启动rpc服务器
   *
   */
  void Start() {
    if (std::atomic_exchange(&start_flag_, true)) return;

    auto self = shared_from_this();
    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(rpc_svr_acceptor_co);

          while (run_flag_) {
            try {
              // 如果链接数达到上限，则等待一段时间再试
              if (session_ptr_list_.size() >= cfg_.max_session_num) {
                acceptor_timer_.expires_after(cfg_.mgr_timer_dt);
                co_await acceptor_timer_.async_wait(boost::asio::use_awaitable);
                continue;
              }

              auto session_ptr = std::make_shared<RpcServer::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_, io_ptr_, func_map_ptr_);
              co_await acceptor_.async_accept(session_ptr->Socket(), boost::asio::use_awaitable);
              session_ptr->Start();

              session_ptr_list_.emplace_back(session_ptr);

            } catch (const std::exception& e) {
              DBG_PRINT("rpc svr accept connection get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);

    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(rpc_svr_timer_co);
          while (run_flag_) {
            try {
              mgr_timer_.expires_after(cfg_.mgr_timer_dt);
              co_await mgr_timer_.async_wait(boost::asio::use_awaitable);

              for (auto itr = session_ptr_list_.begin(); itr != session_ptr_list_.end();) {
                if ((*itr)->IsRunning())
                  ++itr;
                else
                  session_ptr_list_.erase(itr++);
              }
            } catch (const std::exception& e) {
              DBG_PRINT("rpc svr timer get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);
  }

  /**
   * @brief 停止rpc服务器
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(rpc_svr_stop_co);

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
              DBG_PRINT("rpc svr stop get exception at step %u, exception info: %s", stop_step, e.what());
              ++stop_step;
            }
          }

          for (auto& session_ptr : session_ptr_list_)
            session_ptr->Stop();

          session_ptr_list_.clear();
        });
  }

 private:
  // 包头结构：| 2byte magicnum | 2byte headlen | 4byte msglen |
  static const size_t HEAD_SIZE = 8;
  static const char HEAD_BYTE_1 = 'Y';
  static const char HEAD_BYTE_2 = 'T';

  struct MsgContext {
    MsgContext(uint32_t input_req_id, std::vector<char>& input_req_buf, uint32_t input_req_pos)
        : req_id(input_req_id),
          req_buf(input_req_buf),
          req_pos(input_req_pos) {}

    ~MsgContext() {}

    uint32_t req_id;

    std::shared_ptr<Context> ctx_ptr;

    const std::vector<char>& req_buf;
    const uint32_t req_pos = 0;

    BufferVec rsp_buf_vec;
  };

  using AdapterFunc = std::function<boost::asio::awaitable<void>(RpcServer::MsgContext&)>;

  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : max_no_data_duration(cfg.max_no_data_duration) {}

    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(const boost::asio::strand<boost::asio::io_context::executor_type>& session_strand,
            const std::shared_ptr<const RpcServer::SessionCfg>& session_cfg_ptr,
            const std::shared_ptr<boost::asio::io_context>& io_ptr,
            const std::shared_ptr<std::unordered_map<std::string, AdapterFunc>>& func_map_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          sock_(session_strand_),
          send_sig_timer_(session_strand_),
          timer_(session_strand_),
          io_ptr_(io_ptr),
          func_map_ptr_(func_map_ptr) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void Start() {
      auto self = shared_from_this();

      // 发送协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_svr_session_send_co);

            try {
              while (run_flag_) {
                while (!send_buffer_vec_.Vec().empty()) {
                  BufferVec tmp_send_buffer_vec;
                  tmp_send_buffer_vec.Swap(send_buffer_vec_);

                  size_t write_data_size = co_await boost::asio::async_write(sock_, tmp_send_buffer_vec.GetAsioConstBufferVec(), boost::asio::use_awaitable);
                  DBG_PRINT("rpc svr session async write %llu bytes", write_data_size);
                }

                try {
                  send_sig_timer_.expires_at(std::chrono::steady_clock::time_point::max());
                  co_await send_sig_timer_.async_wait(boost::asio::use_awaitable);
                } catch (const std::exception& e) {
                  DBG_PRINT("rpc svr session timer canceled, exception info: %s", e.what());
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("rpc svr session send co get exception and exit, exception info: %s", e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 接收协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_svr_session_recv_co);

            try {
              std::vector<char> head_buf(HEAD_SIZE);
              boost::asio::mutable_buffer asio_head_buf(head_buf.data(), HEAD_SIZE);
              while (run_flag_) {
                // 接收固定包头
                size_t read_data_size = co_await boost::asio::async_read(sock_, asio_head_buf, boost::asio::transfer_exactly(HEAD_SIZE), boost::asio::use_awaitable);
                DBG_PRINT("rpc svr session async read %llu bytes for head", read_data_size);
                tick_has_data_ = true;

                if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
                  throw std::runtime_error("Get an invalid head.");

                // 接收pb包头+pb业务包
                uint32_t pb_msg_len = GetUint16FromBuf(&head_buf[4]);

                // msg长度为0表示是心跳包
                if (pb_msg_len == 0) [[unlikely]]
                  continue;

                std::vector<char> req_buf(pb_msg_len);

                read_data_size = co_await boost::asio::async_read(sock_, boost::asio::buffer(req_buf, pb_msg_len), boost::asio::transfer_exactly(pb_msg_len), boost::asio::use_awaitable);
                DBG_PRINT("rpc svr session async read %llu bytes for pb head", read_data_size);

                // 处理数据，需要post到整个ioctx上
                auto handle = [self, pb_head_len = GetUint16FromBuf(&head_buf[2]), req_buf{std::move(req_buf)}]() mutable -> boost::asio::awaitable<void> {
                  try {
                    ytrpchead::ReqHead req_head;
                    req_head.ParseFromArray(req_buf.data(), pb_head_len);

                    MsgContext msg_ctx(req_head.req_id(), req_buf, pb_head_len);

                    // 查func
                    auto finditr = self->func_map_ptr_->find(req_head.func());
                    if (finditr != self->func_map_ptr_->end()) {
                      // 调用func
                      msg_ctx.ctx_ptr = std::make_shared<Context>();
                      msg_ctx.ctx_ptr->SetDeadline(std::chrono::system_clock::time_point(std::chrono::milliseconds(req_head.ddl_ms())));
                      msg_ctx.ctx_ptr->ContextKv() = std::map<std::string, std::string>(req_head.context_kv().begin(), req_head.context_kv().end());

                      co_await finditr->second(msg_ctx);

                    } else {
                      BufferVecZeroCopyOutputStream os(msg_ctx.rsp_buf_vec);
                      char* head_buf = static_cast<char*>(os.InitHead(RpcServer::HEAD_SIZE));
                      head_buf[0] = RpcServer::HEAD_BYTE_1;
                      head_buf[1] = RpcServer::HEAD_BYTE_2;

                      ytrpchead::RspHead rsp_head;
                      rsp_head.set_req_id(msg_ctx.req_id);
                      rsp_head.set_ret_code(static_cast<int32_t>(StatusCode::NOT_FOUND));

                      rsp_head.SerializeToZeroCopyStream(&os);
                      SetBufFromUint16(&head_buf[2], static_cast<uint16_t>(os.ByteCount() - RpcServer::HEAD_SIZE));
                      SetBufFromUint32(&head_buf[4], static_cast<uint32_t>(os.ByteCount() - RpcServer::HEAD_SIZE));
                      msg_ctx.rsp_buf_vec.CommitLastBuf(os.LastBufSize());
                    }

                    boost::asio::dispatch(
                        self->session_strand_,
                        [self, rsp_buf_vec{std::move(msg_ctx.rsp_buf_vec)}]() mutable {
                          self->send_buffer_vec_.Merge(rsp_buf_vec);
                          self->send_sig_timer_.cancel();
                        });

                  } catch (const std::exception& e) {
                    DBG_PRINT("rpc svr session handle data co get exception and exit, exception info: %s", e.what());
                  }

                  co_return;
                };

                boost::asio::post(
                    *io_ptr_,
                    [self, handle{std::move(handle)}]() mutable {
                      boost::asio::co_spawn(
                          *(self->io_ptr_),
                          std::move(handle),
                          boost::asio::detached);
                    });
              }
            } catch (const std::exception& e) {
              DBG_PRINT("rpc svr session recv co get exception and exit, exception info: %s", e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_svr_session_timer_co);

            try {
              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->max_no_data_duration);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                } else {
                  DBG_PRINT("rpc svr session exit due to timeout(%llums), addr %s.", std::chrono::duration_cast<std::chrono::milliseconds>(session_cfg_ptr_->max_no_data_duration).count(), TcpEp2Str(sock_.remote_endpoint()).c_str());
                  break;
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("rpc svr session timer get exception and exit, addr %s, exception %s", TcpEp2Str(sock_.remote_endpoint()).c_str(), e.what());
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
          session_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(rpc_svr_session_stop_co);

            uint32_t stop_step = 1;
            while (stop_step) {
              try {
                switch (stop_step) {
                  case 1:
                    send_sig_timer_.cancel();
                    ++stop_step;
                  case 2:
                    timer_.cancel();
                    ++stop_step;
                  case 3:
                    sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    ++stop_step;
                  case 4:
                    sock_.cancel();
                    ++stop_step;
                  case 5:
                    sock_.close();
                    ++stop_step;
                  case 6:
                    sock_.release();
                    ++stop_step;
                  default:
                    stop_step = 0;
                    break;
                }
              } catch (const std::exception& e) {
                DBG_PRINT("rpc svr session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    boost::asio::ip::tcp::socket& Socket() { return sock_; }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const RpcServer::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer send_sig_timer_;
    boost::asio::steady_timer timer_;

    std::shared_ptr<boost::asio::io_context> io_ptr_;
    const std::shared_ptr<const std::unordered_map<std::string, AdapterFunc>> func_map_ptr_;
    bool tick_has_data_ = false;
    BufferVec send_buffer_vec_;
  };

 private:
  const RpcServer::Cfg cfg_;
  std::atomic_bool start_flag_ = false;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const RpcServer::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;  // session池操作strand
  boost::asio::ip::tcp::acceptor acceptor_;                                 // 监听器
  boost::asio::steady_timer acceptor_timer_;                                // 连接满时监听器的sleep定时器
  boost::asio::steady_timer mgr_timer_;                                     // 管理session池的定时器
  std::list<std::shared_ptr<RpcServer::Session>> session_ptr_list_;         // session池

  std::list<std::shared_ptr<RpcService>> service_ptr_list_;
  std::shared_ptr<std::unordered_map<std::string, AdapterFunc>> func_map_ptr_;
};

class RpcService {
 protected:
  RpcService() {}
  virtual ~RpcService() {}

  template <typename ReqType, typename RspType>
  void RegisterRpcServiceFunc(
      const std::string& func_name,
      const std::function<boost::asio::awaitable<Status>(const std::shared_ptr<Context>& ctx_ptr, const ReqType& req, RspType& rsp)>& func) {
    func_map_.emplace(
        func_name,
        [func](RpcServer::MsgContext& msg_ctx) -> boost::asio::awaitable<void> {
          BufferVecZeroCopyOutputStream os(msg_ctx.rsp_buf_vec);
          char* head_buf = static_cast<char*>(os.InitHead(RpcServer::HEAD_SIZE));
          head_buf[0] = RpcServer::HEAD_BYTE_1;
          head_buf[1] = RpcServer::HEAD_BYTE_2;

          ytrpchead::RspHead rsp_head;
          rsp_head.set_req_id(msg_ctx.req_id);

          ReqType req;
          if (!req.ParseFromArray(msg_ctx.req_buf.data() + msg_ctx.req_pos, msg_ctx.req_buf.size() - msg_ctx.req_pos)) [[unlikely]] {
            rsp_head.set_ret_code(static_cast<int32_t>(StatusCode::SVR_PARSE_REQ_FAILED));

            rsp_head.SerializeToZeroCopyStream(&os);
            SetBufFromUint16(&head_buf[2], static_cast<uint16_t>(os.ByteCount() - RpcServer::HEAD_SIZE));
            SetBufFromUint32(&head_buf[4], static_cast<uint32_t>(os.ByteCount() - RpcServer::HEAD_SIZE));
            msg_ctx.rsp_buf_vec.CommitLastBuf(os.LastBufSize());

            co_return;
          }

          RspType rsp;
          const Status& ret_status = co_await func(msg_ctx.ctx_ptr, req, rsp);

          rsp_head.set_ret_code(static_cast<int32_t>(ret_status.Ret()));
          rsp_head.set_func_ret_code(static_cast<int32_t>(ret_status.FuncRet()));
          rsp_head.set_func_ret_msg(ret_status.FuncRetMsg());

          rsp_head.SerializeToZeroCopyStream(&os);
          SetBufFromUint16(&head_buf[2], static_cast<uint16_t>(os.ByteCount() - RpcServer::HEAD_SIZE));

          rsp.SerializeToZeroCopyStream(&os);
          SetBufFromUint32(&head_buf[4], static_cast<uint32_t>(os.ByteCount() - RpcServer::HEAD_SIZE));

          msg_ctx.rsp_buf_vec.CommitLastBuf(os.LastBufSize());

          co_return;
        });
  }

 public:
  const std::unordered_map<std::string, RpcServer::AdapterFunc>& FuncMap() const {
    return func_map_;
  }

 private:
  std::unordered_map<std::string, RpcServer::AdapterFunc> func_map_;
};

}  // namespace ytrpc
}  // namespace ytlib
