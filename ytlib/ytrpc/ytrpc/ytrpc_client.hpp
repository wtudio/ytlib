#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

#include "ytrpc_context.hpp"
#include "ytrpc_status.hpp"

#include "Head.pb.h"

namespace ytlib {
namespace ytrpc {

class RpcClient : public std::enable_shared_from_this<RpcClient> {
 public:
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
   * @brief rpc客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  RpcClient(std::shared_ptr<boost::asio::io_context> io_ptr, const RpcClient::Cfg& cfg)
      : cfg_(RpcClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const RpcClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~RpcClient() {}

  RpcClient(const RpcClient&) = delete;             ///< no copy
  RpcClient& operator=(const RpcClient&) = delete;  ///< no copy

  /**
   * @brief 调用rpc
   *
   * @param func_name 方法名
   * @param ctx 上下文
   * @param req_buf req序列化后的buf
   * @param rsp_buf 供rsp序列化的buf
   * @return boost::asio::awaitable<Status>
   */
  boost::asio::awaitable<Status> Invoke(const std::string& func_name, std::shared_ptr<Context> ctx,
                                        std::shared_ptr<boost::asio::streambuf> req_buf,
                                        std::shared_ptr<boost::asio::streambuf>& rsp_buf) {
    return boost::asio::co_spawn(
        mgr_strand_,
        [this, &func_name, ctx, req_buf, &rsp_buf]() -> boost::asio::awaitable<Status> {
          if (!run_flag_) [[unlikely]]
            co_return Status(StatusCode::CLI_IS_NOT_RUNNING);

          if (!session_ptr_ || !session_ptr_->IsRunning()) [[unlikely]] {
            session_ptr_ = std::make_shared<RpcClient::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_);
            session_ptr_->Start();
          }

          co_return co_await session_ptr_->Invoke(func_name, ctx, req_buf, rsp_buf);
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
          ASIO_DEBUG_HANDLE(rpc_cli_stop_co);
          if (session_ptr_) {
            session_ptr_->Stop();
            session_ptr_.reset();
          }
        });
  }

 private:
  // 包头结构：| 2byte magicnum | 2byte headlen | 4byte msglen |
  static const size_t HEAD_SIZE = 8;
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
    struct MsgRecorder {
      MsgRecorder(boost::asio::strand<boost::asio::io_context::executor_type> session_strand)
          : recv_sig_timer(session_strand) {}

      boost::asio::system_timer recv_sig_timer;
      Status ret_status;
      std::shared_ptr<boost::asio::streambuf> rsp_buf;
    };

    Session(boost::asio::strand<boost::asio::io_context::executor_type> session_strand,
            std::shared_ptr<const RpcClient::SessionCfg> session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          sock_(session_strand_),
          sig_timer_(session_strand_) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    boost::asio::awaitable<Status> Invoke(const std::string& func_name, std::shared_ptr<Context> ctx,
                                          std::shared_ptr<boost::asio::streambuf> req_buf,
                                          std::shared_ptr<boost::asio::streambuf>& rsp_buf) {
      return boost::asio::co_spawn(
          session_strand_,
          [this, &func_name, ctx, req_buf, &rsp_buf]() -> boost::asio::awaitable<Status> {
            ASIO_DEBUG_HANDLE(rpc_cli_session_invoke_co);

            if (ctx->IsDone()) [[unlikely]]
              co_return Status(StatusCode::CTX_DONE);

            Status ret_status;
            uint32_t cur_req_id = ++req_id_;
            try {
              ytrpchead::ReqHead req_head;
              req_head.set_req_id(cur_req_id);
              req_head.set_func(func_name);
              req_head.set_ddl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(ctx->Deadline().time_since_epoch()).count());
              (*req_head.mutable_context_kv()) = google::protobuf::Map<std::string, std::string>(ctx->ContextKv().begin(), ctx->ContextKv().end());

              std::shared_ptr<boost::asio::streambuf> req_head_buf = std::make_shared<boost::asio::streambuf>();
              std::ostream os(req_head_buf.get());
              if (!req_head.SerializeToOstream(&os)) [[unlikely]]
                throw std::runtime_error("Req head serialize failed.");

              auto empalce_ret = msg_recorder_map_.emplace(cur_req_id, session_strand_);
              if (!empalce_ret.second) [[unlikely]]
                throw std::runtime_error("Insert into msg recorder map failed.");

              data_list.emplace_back(req_head_buf, req_buf);
              sig_timer_.cancel();

              MsgRecorder& msg_recorder = empalce_ret.first->second;
              bool recv_flag = true;
              try {
                msg_recorder.recv_sig_timer.expires_at(ctx->Deadline());
                co_await msg_recorder.recv_sig_timer.async_wait(boost::asio::use_awaitable);
                recv_flag = false;
              } catch (const std::exception& e) {
                DBG_PRINT("rpc cli session recv sig timer canceled, exception info: %s", e.what());
              }

              if (!recv_flag) [[unlikely]] {
                ret_status = Status(StatusCode::TIMEOUT);
                ctx->DoTimeout("call " + func_name + " timeout");
              } else {
                rsp_buf = msg_recorder.rsp_buf;
                ret_status = msg_recorder.ret_status;
                if (ret_status.Ret() != StatusCode::OK) [[unlikely]]
                  ctx->DoCallFailed("call " + func_name + "failed, " + ret_status.ToString());
              }

            } catch (const std::exception& e) {
              DBG_PRINT("rpc cli session invoke co get exception and exit, exception info: %s", e.what());
              ret_status = Status(StatusCode::UNKNOWN);
            }

            auto finditr = msg_recorder_map_.find(cur_req_id);
            if (finditr != msg_recorder_map_.end())
              msg_recorder_map_.erase(finditr);

            co_return ret_status;
          },
          boost::asio::use_awaitable);
    }

    void Start() {
      auto self = shared_from_this();

      // 启动协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_cli_session_start_co);

            try {
              DBG_PRINT("rpc cli session start create a new connect to %s", TcpEp2Str(session_cfg_ptr_->svr_ep).c_str());
              co_await sock_.async_connect(session_cfg_ptr_->svr_ep, boost::asio::use_awaitable);

              // 发送协程
              boost::asio::co_spawn(
                  session_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(rpc_cli_session_send_co);

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
                            SetBufFromUint32(&head_buf[ct * HEAD_SIZE + 4], static_cast<uint32_t>(itr.first->size() + itr.second->size()));
                            data_buf_list.emplace_back(boost::asio::const_buffer(&head_buf[ct * HEAD_SIZE], HEAD_SIZE));
                            ++ct;

                            data_buf_list.emplace_back(itr.first->data());
                            data_buf_list.emplace_back(itr.second->data());
                          }

                          size_t write_data_size = co_await boost::asio::async_write(sock_, data_buf_list, boost::asio::use_awaitable);
                          DBG_PRINT("rpc cli session async write %llu bytes", write_data_size);
                        }

                        bool heartbeat_flag = false;
                        try {
                          sig_timer_.expires_after(session_cfg_ptr_->timer_dt);
                          co_await sig_timer_.async_wait(boost::asio::use_awaitable);
                          heartbeat_flag = true;
                        } catch (const std::exception& e) {
                          DBG_PRINT("rpc cli session timer canceled, exception info: %s", e.what());
                        }

                        if (heartbeat_flag) {
                          // 心跳包仅用来保活，不传输业务/管理信息
                          const static char heartbeat_pkg[HEAD_SIZE] = {HEAD_BYTE_1, HEAD_BYTE_2, 0, 0, 0, 0, 0, 0};

                          size_t write_data_size = co_await boost::asio::async_write(sock_, boost::asio::const_buffer(heartbeat_pkg, HEAD_SIZE), boost::asio::use_awaitable);
                          DBG_PRINT("cs cli session async write %llu bytes for heartbeat", write_data_size);
                        }
                      }
                    } catch (const std::exception& e) {
                      DBG_PRINT("rpc cli session send co get exception and exit, exception info: %s", e.what());
                    }

                    Stop();

                    co_return;
                  },
                  boost::asio::detached);

              // 接收协程
              boost::asio::co_spawn(
                  session_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(rpc_cli_session_recv_co);

                    try {
                      std::vector<char> head_buf(HEAD_SIZE);
                      while (run_flag_) {
                        // 接收固定包头
                        size_t read_data_size = co_await boost::asio::async_read(sock_, boost::asio::buffer(head_buf, HEAD_SIZE), boost::asio::transfer_exactly(HEAD_SIZE), boost::asio::use_awaitable);
                        DBG_PRINT("rpc cli session async read %llu bytes for head", read_data_size);

                        if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
                          throw std::runtime_error("Get an invalid head.");

                        // 接收pb包头+pb业务包
                        uint32_t pb_msg_len = GetUint16FromBuf(&head_buf[4]);

                        std::shared_ptr<boost::asio::streambuf> msg_buf = std::make_shared<boost::asio::streambuf>();

                        read_data_size = co_await boost::asio::async_read(sock_, msg_buf->prepare(pb_msg_len), boost::asio::transfer_exactly(pb_msg_len), boost::asio::use_awaitable);
                        DBG_PRINT("rpc cli session async read %llu bytes for pb head", read_data_size);

                        if (read_data_size != pb_msg_len) [[unlikely]]
                          throw std::runtime_error("Get an invalid msg.");

                        msg_buf->commit(pb_msg_len);

                        uint16_t pb_head_len = GetUint16FromBuf(&head_buf[2]);
                        ytrpchead::RspHead rsp_head;
                        rsp_head.ParseFromArray(msg_buf->data().data(), pb_head_len);
                        msg_buf->consume(pb_head_len);

                        auto finditr = msg_recorder_map_.find(rsp_head.req_id());
                        if (finditr == msg_recorder_map_.end()) [[unlikely]] {
                          DBG_PRINT("rpc cli session get a no owner pkg, req id:", rsp_head.req_id());
                          continue;
                        }

                        finditr->second.ret_status = Status(
                            static_cast<StatusCode>(rsp_head.ret_code()),
                            rsp_head.func_ret_code(),
                            rsp_head.func_ret_msg());
                        finditr->second.rsp_buf = msg_buf;
                        finditr->second.recv_sig_timer.cancel();
                      }

                    } catch (const std::exception& e) {
                      DBG_PRINT("rpc cli session recv co get exception and exit, exception info: %s", e.what());
                    }

                    Stop();

                    co_return;
                  },
                  boost::asio::detached);

            } catch (const std::exception& e) {
              DBG_PRINT("rpc cli session start co get exception and exit, exception info: %s", e.what());
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
            ASIO_DEBUG_HANDLE(rpc_cli_session_stop_co);

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
                DBG_PRINT("rpc cli session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const RpcClient::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer sig_timer_;

    uint32_t req_id_ = 0;
    std::map<uint32_t, MsgRecorder> msg_recorder_map_;
    std::list<std::pair<std::shared_ptr<boost::asio::streambuf>, std::shared_ptr<boost::asio::streambuf>>> data_list;
  };

 private:
  const RpcClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const RpcClient::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::shared_ptr<RpcClient::Session> session_ptr_;
};

class RpcServiceProxy {
 protected:
  explicit RpcServiceProxy(std::shared_ptr<RpcClient> client_ptr) : client_ptr_(client_ptr) {}
  virtual ~RpcServiceProxy() {}

  template <typename ReqType, typename RspType>
  boost::asio::awaitable<Status> Invoke(const std::string& func_name, std::shared_ptr<Context> ctx, const ReqType& req, RspType& rsp) {
    std::shared_ptr<boost::asio::streambuf> req_buf = std::make_shared<boost::asio::streambuf>();
    std::ostream os(req_buf.get());
    if (!req.SerializeToOstream(&os)) [[unlikely]]
      co_return Status(StatusCode::CLI_SERIALIZE_REQ_FAILED);

    std::shared_ptr<boost::asio::streambuf> rsp_buf;
    const Status& ret_status = co_await client_ptr_->Invoke(func_name, ctx, req_buf, rsp_buf);

    if (ret_status.Ret() != StatusCode::OK) [[unlikely]]
      co_return ret_status;

    if (!rsp_buf) [[unlikely]]
      co_return Status(StatusCode::CLI_PARSE_RSP_FAILED);

    std::istream is(rsp_buf.get());
    if (!rsp.ParseFromIstream(&is)) [[unlikely]]
      co_return Status(StatusCode::CLI_PARSE_RSP_FAILED);

    co_return ret_status;
  }

 private:
  std::shared_ptr<RpcClient> client_ptr_;
};

}  // namespace ytrpc
}  // namespace ytlib
