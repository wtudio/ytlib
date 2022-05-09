#pragma once

#include <concepts>
#include <list>
#include <map>
#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

#include "ytrpc_context.hpp"
#include "ytrpc_status.hpp"

#include "Head.pb.h"

namespace ytlib {
namespace ytrpc {

class RpcService {
  friend class RpcServer;

 protected:
  RpcService() {}
  virtual ~RpcService() {}

  template <typename ReqType, typename RspType>
  void RegisterRpcServiceFunc(
      const std::string& func_name,
      const std::function<boost::asio::awaitable<Status>(std::shared_ptr<Context> ctx, const ReqType& req, RspType& rsp)>& func) {
    func_map_.emplace(
        func_name,
        [func](std::shared_ptr<Context> ctx, std::shared_ptr<boost::asio::streambuf> req_buf, std::shared_ptr<boost::asio::streambuf>& rsp_buf) -> boost::asio::awaitable<Status> {
          ReqType req;
          std::istream is(req_buf.get());
          if (!req.ParseFromIstream(&is)) [[unlikely]]
            co_return Status(StatusCode::SVR_PARSE_REQ_FAILED);

          RspType rsp;
          const Status& ret_status = co_await func(ctx, req, rsp);

          rsp_buf = std::make_shared<boost::asio::streambuf>();
          std::ostream os(rsp_buf.get());
          if (!rsp.SerializeToOstream(&os)) [[unlikely]]
            co_return Status(StatusCode::SVR_SERIALIZE_RSP_FAILED);

          co_return ret_status;
        });
  }

 private:
  using AdapterFunc = std::function<boost::asio::awaitable<Status>(std::shared_ptr<Context>, std::shared_ptr<boost::asio::streambuf>, std::shared_ptr<boost::asio::streambuf>&)>;

  std::map<std::string, AdapterFunc> func_map_;
};

class RpcServer : public std::enable_shared_from_this<RpcServer> {
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
        func_map_ptr_(std::make_shared<std::map<std::string, RpcService::AdapterFunc>>()) {}

  ~RpcServer() {}

  RpcServer(const RpcServer&) = delete;             ///< no copy
  RpcServer& operator=(const RpcServer&) = delete;  ///< no copy

  template <std::derived_from<RpcService> ServiceType>
  void RegisterService(std::shared_ptr<ServiceType> service_ptr) {
    std::shared_ptr<RpcService> rpc_service_ptr = std::static_pointer_cast<RpcService>(service_ptr);
    service_ptr_list_.emplace_back(rpc_service_ptr);
    func_map_ptr_->insert(rpc_service_ptr->func_map_.begin(), rpc_service_ptr->func_map_.end());

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

  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : max_no_data_duration(cfg.max_no_data_duration) {}

    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(boost::asio::strand<boost::asio::io_context::executor_type> session_strand,
            std::shared_ptr<const RpcServer::SessionCfg> session_cfg_ptr,
            std::shared_ptr<boost::asio::io_context> io_ptr,
            std::shared_ptr<std::map<std::string, RpcService::AdapterFunc>> func_map_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          sock_(session_strand_),
          sig_timer_(session_strand_),
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
                while (!data_list.empty()) {
                  std::list<std::pair<std::shared_ptr<boost::asio::streambuf>, std::shared_ptr<boost::asio::streambuf>>> tmp_data_list;
                  tmp_data_list.swap(data_list);

                  std::vector<char> head_buf(tmp_data_list.size() * HEAD_SIZE);

                  std::list<boost::asio::const_buffer> data_buf_list;
                  size_t ct = 0;
                  for (auto& itr : tmp_data_list) {
                    head_buf[ct * HEAD_SIZE] = HEAD_BYTE_1;
                    head_buf[ct * HEAD_SIZE + 1] = HEAD_BYTE_2;
                    SetBufFromUint16(&head_buf[ct * HEAD_SIZE + 2], static_cast<uint16_t>(itr.first->size()));
                    SetBufFromUint32(&head_buf[ct * HEAD_SIZE + 4], static_cast<uint32_t>(itr.first->size() + ((itr.second) ? (itr.second->size()) : 0)));
                    data_buf_list.emplace_back(boost::asio::const_buffer(&head_buf[ct * HEAD_SIZE], HEAD_SIZE));
                    ++ct;

                    data_buf_list.emplace_back(itr.first->data());
                    if (itr.second) data_buf_list.emplace_back(itr.second->data());
                  }

                  tick_has_data_ = true;
                  size_t write_data_size = co_await boost::asio::async_write(sock_, data_buf_list, boost::asio::use_awaitable);
                  DBG_PRINT("rpc svr session async write %llu bytes", write_data_size);
                }

                try {
                  sig_timer_.expires_at(std::chrono::steady_clock::time_point::max());
                  co_await sig_timer_.async_wait(boost::asio::use_awaitable);
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
              while (run_flag_) {
                // 接收固定包头

                size_t read_data_size = co_await boost::asio::async_read(sock_, boost::asio::buffer(head_buf, HEAD_SIZE), boost::asio::transfer_exactly(HEAD_SIZE), boost::asio::use_awaitable);
                DBG_PRINT("rpc svr session async read %llu bytes for head", read_data_size);
                tick_has_data_ = true;

                if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
                  throw std::runtime_error("Get an invalid head.");

                // 接收pb包头+pb业务包
                uint32_t pb_msg_len = GetUint16FromBuf(&head_buf[4]);

                // msg长度为0表示是心跳包
                if (pb_msg_len == 0) [[unlikely]]
                  continue;

                std::shared_ptr<boost::asio::streambuf> msg_buf = std::make_shared<boost::asio::streambuf>();

                read_data_size = co_await boost::asio::async_read(sock_, msg_buf->prepare(pb_msg_len), boost::asio::transfer_exactly(pb_msg_len), boost::asio::use_awaitable);
                DBG_PRINT("rpc svr session async read %llu bytes for pb head", read_data_size);
                tick_has_data_ = true;

                if (read_data_size != pb_msg_len) [[unlikely]]
                  throw std::runtime_error("Get an invalid msg.");

                msg_buf->commit(pb_msg_len);

                // 处理数据，需要post到整个ioctx上
                uint16_t pb_head_len = GetUint16FromBuf(&head_buf[2]);
                boost::asio::post(
                    *io_ptr_,
                    [self, pb_head_len, msg_buf]() {
                      boost::asio::co_spawn(
                          *(self->io_ptr_),
                          [self, pb_head_len, msg_buf]() -> boost::asio::awaitable<void> {
                            // 反序列化pb包头
                            try {
                              ytrpchead::ReqHead req_head;
                              req_head.ParseFromArray(msg_buf->data().data(), pb_head_len);
                              msg_buf->consume(pb_head_len);

                              ytrpchead::RspHead rsp_head;
                              rsp_head.set_req_id(req_head.req_id());

                              std::shared_ptr<boost::asio::streambuf> rsp_buf;

                              // 查func
                              auto finditr = self->func_map_ptr_->find(req_head.func());
                              if (finditr == self->func_map_ptr_->end()) [[unlikely]] {
                                rsp_head.set_ret_code(static_cast<int32_t>(StatusCode::NOT_FOUND));
                              } else {
                                // 调用func
                                auto ctx = std::make_shared<Context>();
                                ctx->SetDeadline(std::chrono::system_clock::time_point(std::chrono::milliseconds(req_head.ddl_ms())));
                                ctx->ContextKv() = std::map<std::string, std::string>(req_head.context_kv().begin(), req_head.context_kv().end());

                                const Status& ret_status = co_await finditr->second(ctx, msg_buf, rsp_buf);
                                rsp_head.set_ret_code(static_cast<int32_t>(ret_status.Ret()));
                                rsp_head.set_func_ret_code(static_cast<int32_t>(ret_status.FuncRet()));
                                rsp_head.set_func_ret_msg(ret_status.FuncRetMsg());
                              }

                              std::shared_ptr<boost::asio::streambuf> rsp_head_buf = std::make_shared<boost::asio::streambuf>();
                              std::ostream os(rsp_head_buf.get());
                              if (!rsp_head.SerializeToOstream(&os)) [[unlikely]]
                                throw std::runtime_error("Rsp head serialize failed.");

                              boost::asio::dispatch(
                                  self->session_strand_,
                                  [self, rsp_head_buf, rsp_buf]() {
                                    self->data_list.emplace_back(rsp_head_buf, rsp_buf);
                                    self->sig_timer_.cancel();
                                  });

                            } catch (const std::exception& e) {
                              DBG_PRINT("rpc svr session handle data co get exception and exit, exception info: %s", e.what());
                            }

                            co_return;
                          },
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
                    sig_timer_.cancel();
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
    boost::asio::steady_timer sig_timer_;
    boost::asio::steady_timer timer_;

    std::list<std::pair<std::shared_ptr<boost::asio::streambuf>, std::shared_ptr<boost::asio::streambuf>>> data_list;
    std::shared_ptr<boost::asio::io_context> io_ptr_;
    const std::shared_ptr<const std::map<std::string, RpcService::AdapterFunc>> func_map_ptr_;
    bool tick_has_data_ = false;
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
  std::shared_ptr<std::map<std::string, RpcService::AdapterFunc>> func_map_ptr_;
};

}  // namespace ytrpc
}  // namespace ytlib
