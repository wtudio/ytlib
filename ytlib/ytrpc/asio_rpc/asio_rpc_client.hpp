/**
 * @file asio_rpc_client.hpp
 * @brief 基于boost.asio的RPC客户端
 * @note
 * @author WT
 * @date 2022-07-13
 */
#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>

#include "ytlib/boost_tools_asio/asio_debug_tools.hpp"
#include "ytlib/boost_tools_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

#include "asio_rpc_context.hpp"
#include "asio_rpc_status.hpp"
#include "ytlib/ytrpc/rpc_util/buffer.hpp"

#include "Head.pb.h"

namespace ytlib {
namespace ytrpc {

class AsioRpcClient : public std::enable_shared_from_this<AsioRpcClient> {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint svr_ep;                                           // 服务端地址
    std::chrono::steady_clock::duration heart_beat_time = std::chrono::seconds(60);  // 心跳包间隔
    uint32_t max_recv_size = 1024 * 1024 * 10;                                       // 回包最大尺寸，最大10m

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
  AsioRpcClient(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioRpcClient::Cfg& cfg)
      : cfg_(AsioRpcClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioRpcClient::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioRpcClient() = default;

  AsioRpcClient(const AsioRpcClient&) = delete;
  AsioRpcClient& operator=(const AsioRpcClient&) = delete;

  boost::asio::awaitable<AsioRpcStatus> Invoke(const std::string& func_name, const std::shared_ptr<const AsioRpcContext>& ctx_ptr, const google::protobuf::Message& req, google::protobuf::Message& rsp) {
    AsioRpcClient::MsgContext msg_ctx(GetNewReqID(), *ctx_ptr);

    if (msg_ctx.ctx.IsDone()) [[unlikely]] {
      co_return AsioRpcStatus(AsioRpcStatus::Code::CANCELLED);
    }

    BufferVecZeroCopyOutputStream os(msg_ctx.req_buf_vec);
    char* head_buf = static_cast<char*>(os.InitHead(AsioRpcClient::HEAD_SIZE));
    head_buf[0] = AsioRpcClient::HEAD_BYTE_1;
    head_buf[1] = AsioRpcClient::HEAD_BYTE_2;

    ReqHead req_head;
    req_head.set_req_id(msg_ctx.req_id);
    req_head.set_func(func_name);
    req_head.set_ddl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(msg_ctx.ctx.Deadline().time_since_epoch()).count());
    (*req_head.mutable_context_kv()) = google::protobuf::Map<std::string, std::string>(msg_ctx.ctx.ContextKv().begin(), msg_ctx.ctx.ContextKv().end());

    if (!req_head.SerializeToZeroCopyStream(&os)) [[unlikely]]
      throw std::runtime_error("Serialize req head failed.");
    SetBufFromUint16(&head_buf[2], static_cast<uint16_t>(os.ByteCount() - AsioRpcClient::HEAD_SIZE));

    if (!req.SerializeToZeroCopyStream(&os)) [[unlikely]]
      throw std::runtime_error("Serialize req failed.");
    SetBufFromUint32(&head_buf[4], static_cast<uint32_t>(os.ByteCount() - AsioRpcClient::HEAD_SIZE));

    msg_ctx.req_buf_vec.CommitLastBuf(os.LastBufSize());

    std::shared_ptr<AsioRpcClient::Session> cur_session_ptr;
    std::atomic_store(&cur_session_ptr, session_ptr_);
    while (!cur_session_ptr || !cur_session_ptr->IsRunning()) {
      if (!run_flag_) [[unlikely]] {
        co_return AsioRpcStatus(AsioRpcStatus::Code::CLI_IS_NOT_RUNNING);
      }

      co_await boost::asio::co_spawn(
          mgr_strand_,
          [this]() -> boost::asio::awaitable<void> {
            if (!run_flag_) [[unlikely]]
              co_return;

            std::shared_ptr<AsioRpcClient::Session> tmp_session_ptr;
            std::atomic_store(&tmp_session_ptr, session_ptr_);

            if (!tmp_session_ptr || !tmp_session_ptr->IsRunning()) {
              tmp_session_ptr = std::make_shared<AsioRpcClient::Session>(io_ptr_, session_cfg_ptr_);
              tmp_session_ptr->Start();
              std::atomic_store(&session_ptr_, tmp_session_ptr);
            }
            co_return;
          },
          boost::asio::use_awaitable);

      std::atomic_store(&cur_session_ptr, session_ptr_);
    }

    co_await cur_session_ptr->Invoke(msg_ctx);

    if (msg_ctx.ret_status.Ret() != AsioRpcStatus::Code::OK) [[unlikely]] {
      msg_ctx.ctx.Done("call " + func_name + "failed, " + msg_ctx.ret_status.ToString());
      co_return std::move(msg_ctx.ret_status);
    }

    if (!rsp.ParseFromArray(msg_ctx.rsp_buf + msg_ctx.rsp_pos, msg_ctx.rsp_buf_len - msg_ctx.rsp_pos)) [[unlikely]]
      co_return AsioRpcStatus(AsioRpcStatus::Code::CLI_PARSE_RSP_FAILED);

    co_return std::move(msg_ctx.ret_status);
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

          std::shared_ptr<AsioRpcClient::Session> cur_session_ptr;
          std::atomic_store(&cur_session_ptr, session_ptr_);

          if (cur_session_ptr) {
            cur_session_ptr->Stop();
            cur_session_ptr.reset();
          }
        });
  }

  /**
   * @brief 获取配置
   *
   * @return const AsioRpcClient::Cfg&
   */
  const AsioRpcClient::Cfg& GetCfg() const { return cfg_; }

 private:
  // 包头结构：| 2byte magic num | 2byte head len | 4byte msg len |
  static constexpr uint32_t HEAD_SIZE = 8;
  static constexpr char HEAD_BYTE_1 = 'Y';
  static constexpr char HEAD_BYTE_2 = 'T';

  uint32_t GetNewReqID() { return ++req_id_; }

  struct MsgContext {
    MsgContext(uint32_t input_req_id, const AsioRpcContext& input_ctx)
        : req_id(input_req_id),
          ctx(input_ctx) {}

    ~MsgContext() = default;

    const uint32_t req_id;
    const AsioRpcContext& ctx;

    BufferVec req_buf_vec;

    AsioRpcStatus ret_status;
    std::shared_ptr<char[]> read_buf_ptr;
    const char* rsp_buf = nullptr;
    uint32_t rsp_buf_len = 0;
    uint32_t rsp_pos = 0;
  };

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
            const std::shared_ptr<const AsioRpcClient::SessionCfg>& session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_socket_strand_(boost::asio::make_strand(*io_ptr)),
          sock_(session_socket_strand_),
          send_sig_timer_(session_socket_strand_),
          session_handle_strand_(boost::asio::make_strand(*io_ptr)) {}

    ~Session() = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    boost::asio::awaitable<void> Invoke(MsgContext& msg_ctx) {
      return boost::asio::co_spawn(
          session_handle_strand_,
          [this, &msg_ctx]() -> boost::asio::awaitable<void> {
            MsgRecorder msg_recorder(msg_ctx, session_handle_strand_, msg_ctx.ctx.Deadline());
            auto empalce_ret = msg_recorder_map_.emplace(msg_ctx.req_id, msg_recorder);

            co_await boost::asio::co_spawn(
                session_socket_strand_,
                [this, &msg_ctx]() -> boost::asio::awaitable<void> {
                  send_buffer_vec_.Merge(msg_ctx.req_buf_vec);
                  send_sig_timer_.cancel();
                  co_return;
                },
                boost::asio::use_awaitable);

            bool recv_flag = true;
            try {
              co_await msg_recorder.recv_sig_timer.async_wait(boost::asio::use_awaitable);
              recv_flag = false;
            } catch (const std::exception& e) {
              DBG_PRINT("rpc cli session recv sig timer canceled, exception info: %s", e.what());
            }

            if (!recv_flag) [[unlikely]] {
              msg_ctx.ret_status = AsioRpcStatus(AsioRpcStatus::Code::TIMEOUT);
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
          session_socket_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_cli_session_start_co);

            try {
              DBG_PRINT("rpc cli session start create a new connect to %s", TcpEp2Str(session_cfg_ptr_->svr_ep).c_str());
              co_await sock_.async_connect(session_cfg_ptr_->svr_ep, boost::asio::use_awaitable);

              // 发送协程
              boost::asio::co_spawn(
                  session_socket_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(rpc_cli_session_send_co);

                    try {
                      while (run_flag_) {
                        while (!send_buffer_vec_.Vec().empty()) {
                          BufferVec tmp_send_buffer_vec;
                          tmp_send_buffer_vec.Swap(send_buffer_vec_);

                          const auto& buffer_vec = tmp_send_buffer_vec.Vec();
                          std::vector<boost::asio::const_buffer> asio_const_buffer_vec;
                          asio_const_buffer_vec.reserve(buffer_vec.size());
                          for (const auto& buffer : buffer_vec) {
                            asio_const_buffer_vec.emplace_back(buffer.first, buffer.second);
                          }

                          size_t write_data_size = co_await boost::asio::async_write(sock_, asio_const_buffer_vec, boost::asio::use_awaitable);
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
                  session_socket_strand_,
                  [this, self]() -> boost::asio::awaitable<void> {
                    ASIO_DEBUG_HANDLE(rpc_cli_session_recv_co);

                    try {
                      constexpr uint32_t min_read_buf_size = 256;
                      uint32_t read_buf_size = min_read_buf_size;
                      uint32_t read_buf_offset = 0;
                      std::shared_ptr<char[]> read_buf_ptr = std::make_shared<char[]>(read_buf_size);

                      while (run_flag_) {
                        // 接收数据
                        size_t read_data_size = co_await boost::asio::async_read(
                            sock_, boost::asio::mutable_buffer(read_buf_ptr.get() + read_buf_offset, read_buf_size - read_buf_offset),
                            [&](const boost::system::error_code& error, size_t bytes_transferred) -> size_t {
                              if (error) [[unlikely]]
                                return 0;

                              bytes_transferred += read_buf_offset;  // buf中实际已经有的数据大小
                              if (bytes_transferred < HEAD_SIZE) return read_buf_size - bytes_transferred;
                              if (read_buf_ptr[0] != HEAD_BYTE_1 || read_buf_ptr[1] != HEAD_BYTE_2) return 0;
                              if (bytes_transferred < (HEAD_SIZE + GetUint32FromBuf(read_buf_ptr.get() + 4))) return read_buf_size - bytes_transferred;
                              return 0;
                            },
                            boost::asio::use_awaitable);
                        DBG_PRINT("rpc cli session async read %llu bytes", read_data_size);

                        read_data_size += read_buf_offset;  // buf中实际数据大小

                        // 根据本次数据接收情况决定下次数据buf大小
                        if (read_data_size < read_buf_size / 2) {
                          read_buf_size = std::max(min_read_buf_size, read_buf_size / 2);
                        } else if (read_data_size >= read_buf_size) {
                          read_buf_size = std::min(session_cfg_ptr_->max_recv_size, read_buf_size * 2);
                        }

                        uint32_t cur_handle_pos = 0;
                        while (true) {
                          const uint32_t cur_unhandle_size = read_data_size - cur_handle_pos;

                          if (cur_unhandle_size < HEAD_SIZE) break;

                          if (read_buf_ptr[cur_handle_pos] != HEAD_BYTE_1 || read_buf_ptr[cur_handle_pos + 1] != HEAD_BYTE_2) [[unlikely]]
                            throw std::runtime_error("Get an invalid head.");

                          // pb包头+pb业务包大小
                          const uint32_t pb_msg_len = GetUint32FromBuf(read_buf_ptr.get() + cur_handle_pos + 4);

                          if ((HEAD_SIZE + pb_msg_len) > session_cfg_ptr_->max_recv_size) [[unlikely]]
                            throw std::runtime_error("Msg too large.");

                          // 如果HEAD_SIZE + pb_msg_len大于read_buf_size，则下次接收时buf会扩大
                          if (cur_unhandle_size < (HEAD_SIZE + pb_msg_len)) {
                            read_buf_size = std::max(read_buf_size, (HEAD_SIZE + pb_msg_len));
                            break;
                          }

                          const char* rsp_buf = read_buf_ptr.get() + cur_handle_pos + HEAD_SIZE;
                          const uint16_t pb_head_len = GetUint16FromBuf(read_buf_ptr.get() + cur_handle_pos + 2);

                          boost::asio::post(
                              session_handle_strand_,
                              [this, self, read_buf_ptr, rsp_buf, pb_head_len, pb_msg_len]() {
                                ASIO_DEBUG_HANDLE(rpc_cli_session_recv_handle_co);

                                try {
                                  RspHead rsp_head;
                                  if (!rsp_head.ParseFromArray(rsp_buf, pb_head_len)) [[unlikely]]
                                    throw std::runtime_error("Parse rsp head failed.");

                                  auto finditr = msg_recorder_map_.find(rsp_head.req_id());
                                  if (finditr == msg_recorder_map_.end()) [[unlikely]] {
                                    DBG_PRINT("rpc cli session get a no owner pkg, req id:", rsp_head.req_id());
                                    return;
                                  }

                                  finditr->second.msg_ctx.ret_status = AsioRpcStatus(
                                      static_cast<AsioRpcStatus::Code>(rsp_head.ret_code()),
                                      rsp_head.func_ret_code(),
                                      rsp_head.func_ret_msg());
                                  finditr->second.msg_ctx.read_buf_ptr = read_buf_ptr;
                                  finditr->second.msg_ctx.rsp_buf = rsp_buf;
                                  finditr->second.msg_ctx.rsp_buf_len = pb_msg_len;
                                  finditr->second.msg_ctx.rsp_pos = pb_head_len;
                                  finditr->second.recv_sig_timer.cancel();
                                } catch (const std::exception& e) {
                                  DBG_PRINT("rpc cli session recv handle co get exception and exit, exception info: %s", e.what());
                                }
                              });

                          cur_handle_pos += (HEAD_SIZE + pb_msg_len);
                        }

                        read_buf_offset = read_data_size - cur_handle_pos;

                        if (read_buf_offset) {
                          // todo：考虑使用vectorbuf，不用拷贝一次
                          std::shared_ptr<char[]> last_read_buf_ptr = read_buf_ptr;
                          read_buf_ptr = std::make_shared<char[]>(read_buf_size);
                          memcpy(read_buf_ptr.get(), last_read_buf_ptr.get() + cur_handle_pos, read_buf_offset);
                        } else {
                          read_buf_ptr = std::make_shared<char[]>(read_buf_size);
                        }
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
          session_socket_strand_,
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
      MsgRecorder(MsgContext& input_msg_ctx,
                  const boost::asio::strand<boost::asio::io_context::executor_type>& session_strand,
                  const std::chrono::system_clock::time_point& ddl)
          : msg_ctx(input_msg_ctx),
            recv_sig_timer(session_strand, ddl) {}
      ~MsgRecorder() = default;

      MsgContext& msg_ctx;
      boost::asio::system_timer recv_sig_timer;
    };

    std::shared_ptr<const AsioRpcClient::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;

    boost::asio::strand<boost::asio::io_context::executor_type> session_socket_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer send_sig_timer_;
    BufferVec send_buffer_vec_;

    boost::asio::strand<boost::asio::io_context::executor_type> session_handle_strand_;
    std::unordered_map<uint32_t, MsgRecorder&> msg_recorder_map_;
  };

 private:
  const AsioRpcClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  std::shared_ptr<const AsioRpcClient::SessionCfg> session_cfg_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::shared_ptr<AsioRpcClient::Session> session_ptr_;

  std::atomic_uint32_t req_id_ = 0;
};

class AsioRpcServiceProxy {
 protected:
  explicit AsioRpcServiceProxy(const std::shared_ptr<AsioRpcClient>& client_ptr) : client_ptr_(client_ptr) {}
  virtual ~AsioRpcServiceProxy() = default;

  boost::asio::awaitable<AsioRpcStatus> Invoke(const std::string& func_name, const std::shared_ptr<const AsioRpcContext>& ctx_ptr, const google::protobuf::Message& req, google::protobuf::Message& rsp) {
    return client_ptr_->Invoke(func_name, ctx_ptr, req, rsp);
  }

 private:
  std::shared_ptr<AsioRpcClient> client_ptr_;
};

}  // namespace ytrpc
}  // namespace ytlib
