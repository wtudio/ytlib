#pragma once

#include <atomic>
#include <memory>
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

class RpcClient : public std::enable_shared_from_this<RpcClient> {
  friend class RpcServiceProxy;

 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint svr_ep;  // 服务端地址

    std::chrono::steady_clock::duration heart_beat_time = std::chrono::seconds(60);  // 心跳包间隔

    uint32_t max_recv_size = 1024 * 1024 * 10;  // 回包最大尺寸，最大10m

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.heart_beat_time < std::chrono::milliseconds(100)) cfg.heart_beat_time = std::chrono::milliseconds(100);

      return cfg;
    }
  };

  /**
   * @brief rpc客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  RpcClient(const std::shared_ptr<boost::asio::io_context>& io_ptr, const RpcClient::Cfg& cfg)
      : cfg_(RpcClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const RpcClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~RpcClient() {}

  RpcClient(const RpcClient&) = delete;             ///< no copy
  RpcClient& operator=(const RpcClient&) = delete;  ///< no copy

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

  struct MsgContext {
    MsgContext(uint32_t input_req_id, const Context& input_ctx)
        : req_id(input_req_id),
          ctx(input_ctx) {}

    ~MsgContext() {}

    const uint32_t req_id;
    const Context& ctx;

    BufferVec req_buf_vec;

    Status ret_status;
    std::vector<char> rsp_buf;
    uint32_t rsp_pos = 0;
  };

  uint32_t GetNewReqID() { return ++req_id_; }

  boost::asio::awaitable<void> Invoke(MsgContext& msg_ctx) {
    while (!session_ptr_ || !session_ptr_->IsRunning()) {
      if (!run_flag_) [[unlikely]] {
        msg_ctx.ret_status = Status(StatusCode::CLI_IS_NOT_RUNNING);
        co_return;
      }

      co_await boost::asio::co_spawn(
          mgr_strand_,
          [this]() -> boost::asio::awaitable<void> {
            if (!run_flag_) [[unlikely]]
              co_return;

            if (!session_ptr_ || !session_ptr_->IsRunning()) [[unlikely]] {
              session_ptr_ = std::make_shared<RpcClient::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_);
              session_ptr_->Start();
            }
            co_return;
          },
          boost::asio::use_awaitable);
    }

    co_await session_ptr_->Invoke(msg_ctx);
    co_return;
  }

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
    Session(const boost::asio::strand<boost::asio::io_context::executor_type>& session_strand,
            const std::shared_ptr<const RpcClient::SessionCfg>& session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          sock_(session_strand_),
          send_sig_timer_(session_strand_) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    boost::asio::awaitable<void> Invoke(MsgContext& msg_ctx) {
      return boost::asio::co_spawn(
          session_strand_,
          [this, &msg_ctx]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_cli_session_invoke_co);

            MsgRecorder msg_recorder(msg_ctx, session_strand_);
            auto empalce_ret = msg_recorder_map_.emplace(msg_ctx.req_id, msg_recorder);

            send_buffer_vec_.Merge(msg_ctx.req_buf_vec);
            send_sig_timer_.cancel();

            bool recv_flag = true;
            try {
              msg_recorder.recv_sig_timer.expires_at(msg_ctx.ctx.Deadline());
              co_await msg_recorder.recv_sig_timer.async_wait(boost::asio::use_awaitable);
              recv_flag = false;
            } catch (const std::exception& e) {
              DBG_PRINT("rpc cli session recv sig timer canceled, exception info: %s", e.what());
            }

            if (!recv_flag) [[unlikely]] {
              msg_ctx.ret_status = Status(StatusCode::TIMEOUT);
            }

            msg_recorder_map_.erase(empalce_ret.first);

            co_return;
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
                        while (!send_buffer_vec_.Vec().empty()) {
                          BufferVec tmp_send_buffer_vec;
                          tmp_send_buffer_vec.Swap(send_buffer_vec_);

                          size_t write_data_size = co_await boost::asio::async_write(sock_, tmp_send_buffer_vec.GetAsioConstBufferVec(), boost::asio::use_awaitable);
                          DBG_PRINT("rpc cli session async write %llu bytes", write_data_size);
                        }

                        bool heartbeat_flag = false;
                        try {
                          send_sig_timer_.expires_after(session_cfg_ptr_->heart_beat_time);
                          co_await send_sig_timer_.async_wait(boost::asio::use_awaitable);
                          heartbeat_flag = true;
                        } catch (const std::exception& e) {
                          DBG_PRINT("rpc cli session timer canceled, exception info: %s", e.what());
                        }

                        if (heartbeat_flag) {
                          // 心跳包仅用来保活，不传输业务/管理信息
                          const static char heartbeat_pkg[HEAD_SIZE] = {HEAD_BYTE_1, HEAD_BYTE_2, 0, 0, 0, 0, 0, 0};
                          const static boost::asio::const_buffer heartbeat_buf(heartbeat_pkg, HEAD_SIZE);

                          size_t write_data_size = co_await boost::asio::async_write(sock_, heartbeat_buf, boost::asio::use_awaitable);
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
                      boost::asio::mutable_buffer asio_head_buf(head_buf.data(), HEAD_SIZE);
                      while (run_flag_) {
                        // 接收固定包头
                        size_t read_data_size = co_await boost::asio::async_read(sock_, asio_head_buf, boost::asio::transfer_exactly(HEAD_SIZE), boost::asio::use_awaitable);
                        DBG_PRINT("rpc cli session async read %llu bytes for head", read_data_size);

                        if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
                          throw std::runtime_error("Get an invalid head.");

                        // 接收pb包头+pb业务包
                        const uint32_t pb_msg_len = GetUint32FromBuf(&head_buf[4]);

                        if (pb_msg_len > session_cfg_ptr_->max_recv_size) [[unlikely]] {
                          throw std::runtime_error("Msg too large.");
                        }

                        std::vector<char> rsp_buf(pb_msg_len);

                        read_data_size = co_await boost::asio::async_read(sock_, boost::asio::buffer(rsp_buf, pb_msg_len), boost::asio::transfer_exactly(pb_msg_len), boost::asio::use_awaitable);
                        DBG_PRINT("rpc cli session async read %llu bytes for pb head", read_data_size);

                        const uint16_t pb_head_len = GetUint16FromBuf(&head_buf[2]);
                        thread_local ytrpchead::RspHead rsp_head;
                        rsp_head.ParseFromArray(rsp_buf.data(), pb_head_len);

                        auto finditr = msg_recorder_map_.find(rsp_head.req_id());
                        if (finditr == msg_recorder_map_.end()) [[unlikely]] {
                          DBG_PRINT("rpc cli session get a no owner pkg, req id:", rsp_head.req_id());
                          continue;
                        }

                        finditr->second.msg_ctx.ret_status = Status(
                            static_cast<StatusCode>(rsp_head.ret_code()),
                            rsp_head.func_ret_code(),
                            rsp_head.func_ret_msg());
                        finditr->second.msg_ctx.rsp_buf = std::move(rsp_buf);
                        finditr->second.msg_ctx.rsp_pos = pb_head_len;
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
                DBG_PRINT("rpc cli session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    struct MsgRecorder {
      MsgRecorder(MsgContext& input_msg_ctx, const boost::asio::strand<boost::asio::io_context::executor_type>& session_strand)
          : msg_ctx(input_msg_ctx),
            recv_sig_timer(session_strand) {}
      ~MsgRecorder() {}

      MsgContext& msg_ctx;
      boost::asio::system_timer recv_sig_timer;
    };

    std::shared_ptr<const RpcClient::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer send_sig_timer_;

    std::unordered_map<uint32_t, MsgRecorder&> msg_recorder_map_;
    BufferVec send_buffer_vec_;
  };

 private:
  const RpcClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const RpcClient::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::shared_ptr<RpcClient::Session> session_ptr_;

  std::atomic_uint32_t req_id_ = 0;
};

class RpcServiceProxy {
 protected:
  explicit RpcServiceProxy(const std::shared_ptr<RpcClient>& client_ptr) : client_ptr_(client_ptr) {}
  virtual ~RpcServiceProxy() {}

  template <typename ReqType, typename RspType>
  boost::asio::awaitable<Status> Invoke(const std::string& func_name, const std::shared_ptr<const Context>& ctx_ptr, const ReqType& req, RspType& rsp) {
    RpcClient::MsgContext msg_ctx(client_ptr_->GetNewReqID(), *ctx_ptr);

    if (msg_ctx.ctx.IsDone()) [[unlikely]] {
      co_return Status(StatusCode::CANCELLED);
    }

    BufferVecZeroCopyOutputStream os(msg_ctx.req_buf_vec);
    char* head_buf = static_cast<char*>(os.InitHead(RpcClient::HEAD_SIZE));
    head_buf[0] = RpcClient::HEAD_BYTE_1;
    head_buf[1] = RpcClient::HEAD_BYTE_2;

    thread_local ytrpchead::ReqHead req_head;
    req_head.set_req_id(msg_ctx.req_id);
    req_head.set_func(func_name);
    req_head.set_ddl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(msg_ctx.ctx.Deadline().time_since_epoch()).count());
    (*req_head.mutable_context_kv()) = google::protobuf::Map<std::string, std::string>(msg_ctx.ctx.ContextKv().begin(), msg_ctx.ctx.ContextKv().end());

    req_head.SerializeToZeroCopyStream(&os);
    SetBufFromUint16(&head_buf[2], static_cast<uint16_t>(os.ByteCount() - RpcClient::HEAD_SIZE));

    req.SerializeToZeroCopyStream(&os);
    SetBufFromUint32(&head_buf[4], static_cast<uint32_t>(os.ByteCount() - RpcClient::HEAD_SIZE));

    msg_ctx.req_buf_vec.CommitLastBuf(os.LastBufSize());

    co_await client_ptr_->Invoke(msg_ctx);

    if (msg_ctx.ret_status.Ret() != StatusCode::OK) [[unlikely]] {
      msg_ctx.ctx.Done("call " + func_name + "failed, " + msg_ctx.ret_status.ToString());
      co_return std::move(msg_ctx.ret_status);
    }

    if (!rsp.ParseFromArray(msg_ctx.rsp_buf.data() + msg_ctx.rsp_pos, msg_ctx.rsp_buf.size() - msg_ctx.rsp_pos)) [[unlikely]]
      co_return Status(StatusCode::CLI_PARSE_RSP_FAILED);

    co_return std::move(msg_ctx.ret_status);
  }

 private:
  std::shared_ptr<RpcClient> client_ptr_;
};

}  // namespace ytrpc
}  // namespace ytlib
