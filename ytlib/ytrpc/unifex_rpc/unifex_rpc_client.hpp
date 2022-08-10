#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include <boost/asio.hpp>

#include <unifex/async_mutex.hpp>
#include <unifex/task.hpp>

#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/execution/execution_tools.hpp"
#include "ytlib/misc/misc_macro.h"

#include "unifex_rpc_context.hpp"
#include "unifex_rpc_status.hpp"
#include "ytlib/ytrpc/rpc_util/buffer.hpp"

#include "Head.pb.h"

namespace ytlib {
namespace ytrpc {

/**
 * @brief 基于libunifex作为并发接口形式的rpc实现
 * 底层网络仍然使用asio
 */
class UnifexRpcClient : public std::enable_shared_from_this<UnifexRpcClient> {
 public:
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

  UnifexRpcClient(const std::shared_ptr<boost::asio::io_context>& io_ptr, const UnifexRpcClient::Cfg& cfg)
      : cfg_(UnifexRpcClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const UnifexRpcClient::SessionCfg>(cfg_)) {}

  ~UnifexRpcClient() {}

  UnifexRpcClient(const UnifexRpcClient&) = delete;             ///< no copy
  UnifexRpcClient& operator=(const UnifexRpcClient&) = delete;  ///< no copy

  void Invoke(const std::string& func_name, const std::shared_ptr<const UnifexRpcContext>& ctx_ptr,
              const google::protobuf::Message& req, google::protobuf::Message& rsp, std::function<void(UnifexRpcStatus&&)>&& callback) {
    StartDetached(
        unifex::co_invoke([this, &func_name, &ctx_ptr, &req, &rsp]() -> unifex::task<UnifexRpcStatus> {
          UnifexRpcClient::MsgContext msg_ctx(GetNewReqID(), *ctx_ptr);

          if (msg_ctx.ctx.IsDone()) [[unlikely]] {
            co_return UnifexRpcStatus(UnifexRpcStatus::Code::CANCELLED);
          }

          BufferVecZeroCopyOutputStream os(msg_ctx.req_buf_vec);
          char* head_buf = static_cast<char*>(os.InitHead(UnifexRpcClient::HEAD_SIZE));
          head_buf[0] = UnifexRpcClient::HEAD_BYTE_1;
          head_buf[1] = UnifexRpcClient::HEAD_BYTE_2;

          ReqHead req_head;
          req_head.set_req_id(msg_ctx.req_id);
          req_head.set_func(func_name);
          req_head.set_ddl_ms(std::chrono::duration_cast<std::chrono::milliseconds>(msg_ctx.ctx.Deadline().time_since_epoch()).count());
          (*req_head.mutable_context_kv()) = google::protobuf::Map<std::string, std::string>(msg_ctx.ctx.ContextKv().begin(), msg_ctx.ctx.ContextKv().end());

          if (!req_head.SerializeToZeroCopyStream(&os)) [[unlikely]]
            throw std::runtime_error("Serialize req head failed.");
          SetBufFromUint16(&head_buf[2], static_cast<uint16_t>(os.ByteCount() - UnifexRpcClient::HEAD_SIZE));

          if (!req.SerializeToZeroCopyStream(&os)) [[unlikely]]
            throw std::runtime_error("Serialize req failed.");
          SetBufFromUint32(&head_buf[4], static_cast<uint32_t>(os.ByteCount() - UnifexRpcClient::HEAD_SIZE));

          msg_ctx.req_buf_vec.CommitLastBuf(os.LastBufSize());

          std::shared_ptr<UnifexRpcClient::Session> cur_session_ptr = session_ptr_;
          while (!cur_session_ptr || !cur_session_ptr->IsRunning()) {
            if (!run_flag_) [[unlikely]] {
              co_return UnifexRpcStatus(UnifexRpcStatus::Code::CLI_IS_NOT_RUNNING);
            }

            co_await mgr_mutex_.async_lock();
            if (run_flag_) {
              if (!session_ptr_ || !session_ptr_->IsRunning()) {
                session_ptr_ = std::make_shared<UnifexRpcClient::Session>(io_ptr_, session_cfg_ptr_);
                session_ptr_->Start();
              }
            }
            mgr_mutex_.unlock();

            cur_session_ptr = session_ptr_;
          }

          co_await cur_session_ptr->Invoke(msg_ctx);

          if (msg_ctx.ret_status.Ret() != UnifexRpcStatus::Code::OK) [[unlikely]] {
            msg_ctx.ctx.Done("call " + func_name + "failed, " + msg_ctx.ret_status.ToString());
            co_return std::move(msg_ctx.ret_status);
          }

          if (!rsp.ParseFromArray(msg_ctx.rsp_buf.data() + msg_ctx.rsp_pos, msg_ctx.rsp_buf.size() - msg_ctx.rsp_pos)) [[unlikely]]
            co_return UnifexRpcStatus(UnifexRpcStatus::Code::CLI_PARSE_RSP_FAILED);

          co_return std::move(msg_ctx.ret_status);
        }),
        std::move(callback));
  }

  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    StartDetached(unifex::co_invoke([this, self]() -> unifex::task<void> {
      co_await mgr_mutex_.async_lock();
      if (session_ptr_) {
        session_ptr_->Stop();
        session_ptr_.reset();
      }
      mgr_mutex_.unlock();
    }));
  }

  const UnifexRpcClient::Cfg& GetCfg() const { return cfg_; }

 private:
  // 包头结构：| 2byte magicnum | 2byte headlen | 4byte msglen |
  static const size_t HEAD_SIZE = 8;
  static const char HEAD_BYTE_1 = 'Y';
  static const char HEAD_BYTE_2 = 'T';

  uint32_t GetNewReqID() { return ++req_id_; }

  struct MsgContext {
    MsgContext(uint32_t input_req_id, const UnifexRpcContext& input_ctx)
        : req_id(input_req_id),
          ctx(input_ctx) {}

    ~MsgContext() {}

    const uint32_t req_id;
    const UnifexRpcContext& ctx;

    BufferVec req_buf_vec;

    UnifexRpcStatus ret_status;
    std::vector<char> rsp_buf;
    uint32_t rsp_pos = 0;
  };

  template <typename Receiver>
  requires unifex::receiver<Receiver>
  struct RecvSigOperationState {
    template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
    RecvSigOperationState(std::function<void()>& recv_callback, Receiver2&& r)
    noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
        : recv_callback_(recv_callback), receiver_(new Receiver((Receiver2 &&) r)) {}

    void start() noexcept {
      recv_callback_ = [receiver = receiver_]() {
        try {
          unifex::set_value(std::move(*receiver));
        } catch (...) {
          unifex::set_error(std::move(*receiver), std::current_exception());
        }
      };
    }

    std::function<void()>& recv_callback_;

    std::shared_ptr<Receiver> receiver_;
  };

  class RecvSigSender {
   public:
    template <template <typename...> class Variant, template <typename...> class Tuple>
    using value_types = Variant<Tuple<>>;

    template <template <typename...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = false;

    RecvSigSender(std::function<void()>& recv_callback)
        : recv_callback_(recv_callback) {}

    template <typename Receiver>
    RecvSigOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
      return RecvSigOperationState<unifex::remove_cvref_t<Receiver>>(recv_callback_, (Receiver &&) receiver);
    }

   private:
    std::function<void()>& recv_callback_;
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
            const std::shared_ptr<const UnifexRpcClient::SessionCfg>& session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          sock_(*io_ptr) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    unifex::task<void> Invoke(MsgContext& msg_ctx) {
      MsgRecorder msg_recorder(msg_ctx);

      co_await msg_recorder_map_mutex_.async_lock();
      auto empalce_ret = msg_recorder_map_.emplace(msg_ctx.req_id, msg_recorder);
      msg_recorder_map_mutex_.unlock();

      // todo：可以优化到TrySend中，不必使用atomic_bool，省一次加锁
      co_await send_buffer_vec_mutex_.async_lock();
      send_buffer_vec_.Merge(msg_ctx.req_buf_vec);
      send_buffer_vec_mutex_.unlock();

      StartDetached(TrySend());

      co_await msg_recorder.recv_sig;

      co_await msg_recorder_map_mutex_.async_lock();
      msg_recorder_map_.erase(empalce_ret.first);
      msg_recorder_map_mutex_.unlock();

      co_return;
    }

    void Start() {
      auto self = shared_from_this();

      StartDetached(unifex::co_invoke([this, self]() -> unifex::task<void> {
        DBG_PRINT("rpc cli session start create a new connect to %s", TcpEp2Str(session_cfg_ptr_->svr_ep).c_str());
        boost::system::error_code ec = co_await AsyncWrapper<boost::system::error_code>([&](std::function<void(boost::system::error_code)>&& cb) {
          sock_.async_connect(session_cfg_ptr_->svr_ep, std::move(cb));
        });

        if (ec) {
          DBG_PRINT("rpc cli session async connect get error, %s", ec.message().c_str());
          Stop();
          co_return;
        }

        std::vector<char> head_buf(HEAD_SIZE);
        boost::asio::mutable_buffer asio_head_buf(head_buf.data(), HEAD_SIZE);
        while (run_flag_) {
          // 接收固定包头
          auto [ec, read_data_size] = co_await AsyncWrapper<boost::system::error_code, size_t>([&](std::function<void(boost::system::error_code, size_t)>&& cb) {
            boost::asio::async_read(sock_, asio_head_buf, boost::asio::transfer_exactly(HEAD_SIZE), std::move(cb));
          });
          DBG_PRINT("rpc cli session async read %llu bytes for head", read_data_size);

          if (ec) {
            DBG_PRINT("rpc cli session async read get error, %s", ec.message().c_str());
            Stop();
            co_return;
          }

          if (read_data_size != HEAD_SIZE || head_buf[0] != HEAD_BYTE_1 || head_buf[1] != HEAD_BYTE_2) [[unlikely]]
            throw std::runtime_error("Get an invalid head.");

          // 接收pb包头+pb业务包
          const uint32_t pb_msg_len = GetUint32FromBuf(&head_buf[4]);

          if (pb_msg_len > session_cfg_ptr_->max_recv_size) [[unlikely]]
            throw std::runtime_error("Msg too large.");

          std::vector<char> rsp_buf(pb_msg_len);

          std::tie(ec, read_data_size) = co_await AsyncWrapper<boost::system::error_code, size_t>([&](std::function<void(boost::system::error_code, size_t)>&& cb) {
            boost::asio::async_read(sock_, boost::asio::buffer(rsp_buf, pb_msg_len), boost::asio::transfer_exactly(pb_msg_len), std::move(cb));
          });
          DBG_PRINT("rpc cli session async read %llu bytes for pb head", read_data_size);

          if (ec) {
            DBG_PRINT("rpc cli session async read get error, %s", ec.message().c_str());
            Stop();
            co_return;
          }

          size_t pb_head_len = GetUint16FromBuf(&head_buf[2]);

          RspHead rsp_head;
          if (!rsp_head.ParseFromArray(rsp_buf.data(), pb_head_len)) [[unlikely]]
            throw std::runtime_error("Parse rsp head failed.");

          auto finditr = msg_recorder_map_.find(rsp_head.req_id());
          if (finditr == msg_recorder_map_.end()) [[unlikely]] {
            DBG_PRINT("rpc cli session get a no owner pkg, req id:", rsp_head.req_id());
            continue;
          }

          finditr->second.msg_ctx.ret_status = UnifexRpcStatus(
              static_cast<UnifexRpcStatus::Code>(rsp_head.ret_code()),
              rsp_head.func_ret_code(),
              rsp_head.func_ret_msg());
          finditr->second.msg_ctx.rsp_buf = std::move(rsp_buf);
          finditr->second.msg_ctx.rsp_pos = pb_head_len;
          finditr->second.recv_callback();
        }
      }));
    }

    void Stop() {
      if (!std::atomic_exchange(&run_flag_, false)) return;
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    unifex::task<void> TrySend() {
      if (std::atomic_exchange(&sending_flag_, true)) co_return;

      while (run_flag_) {
        BufferVec tmp_send_buffer_vec;

        co_await send_buffer_vec_mutex_.async_lock();
        if (send_buffer_vec_.Vec().empty()) {
          sending_flag_ = false;
          send_buffer_vec_mutex_.unlock();
          co_return;
        }
        tmp_send_buffer_vec.Swap(send_buffer_vec_);
        send_buffer_vec_mutex_.unlock();

        const auto& buffer_vec = tmp_send_buffer_vec.Vec();
        std::vector<boost::asio::const_buffer> asio_const_buffer_vec;
        asio_const_buffer_vec.reserve(buffer_vec.size());
        for (const auto& buffer : buffer_vec) {
          asio_const_buffer_vec.push_back(boost::asio::const_buffer(buffer.first, buffer.second));
        }

        auto [ec, write_data_size] = co_await AsyncWrapper<boost::system::error_code, size_t>([&](std::function<void(boost::system::error_code, size_t)>&& cb) {
          boost::asio::async_write(sock_, asio_const_buffer_vec, std::move(cb));
        });

        if (ec) {
          DBG_PRINT("rpc cli session async write get error, %s", ec.message().c_str());
          sending_flag_ = false;
          Stop();
          co_return;
        }

        DBG_PRINT("rpc cli session async write %llu bytes", write_data_size);
      }

      co_return;
    }

   private:
    struct MsgRecorder {
      MsgRecorder(MsgContext& input_msg_ctx)
          : msg_ctx(input_msg_ctx), recv_sig(recv_callback) {}

      MsgContext& msg_ctx;
      std::function<void()> recv_callback;
      RecvSigSender recv_sig;
    };

    std::shared_ptr<const UnifexRpcClient::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;

    std::atomic_bool sending_flag_ = false;
    boost::asio::ip::tcp::socket sock_;

    unifex::async_mutex send_buffer_vec_mutex_;
    BufferVec send_buffer_vec_;

    unifex::async_mutex msg_recorder_map_mutex_;
    std::unordered_map<uint32_t, MsgRecorder&> msg_recorder_map_;
  };

 private:
  const UnifexRpcClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  std::shared_ptr<const UnifexRpcClient::SessionCfg> session_cfg_ptr_;

  unifex::async_mutex mgr_mutex_;
  std::shared_ptr<UnifexRpcClient::Session> session_ptr_;

  std::atomic_uint32_t req_id_ = 0;
};

// 代表单次请求中的所有数据
template <typename Req, typename Rsp>
struct UnifexRpcMsgContext {
  UnifexRpcMsgContext(const std::shared_ptr<UnifexRpcClient>& client_ptr, const std::string& func_name, const std::shared_ptr<const UnifexRpcContext>& ctx_ptr, const Req& req)
      : client_(*client_ptr), func_name_(func_name), ctx_ptr_(ctx_ptr), req_(req) {}

  UnifexRpcMsgContext(const UnifexRpcMsgContext&) = delete;             ///< no copy
  UnifexRpcMsgContext& operator=(const UnifexRpcMsgContext&) = delete;  ///< no copy

  UnifexRpcClient& client_;

  const std::string& func_name_;
  std::shared_ptr<const UnifexRpcContext> ctx_ptr_;
  const Req& req_;

  Rsp rsp_;
};

template <typename Req, typename Rsp, typename Receiver>
class UnifexRpcOperationState final {
 public:
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  UnifexRpcOperationState(const std::shared_ptr<UnifexRpcMsgContext<Req, Rsp>>& msg_ctx_ptr, Receiver2&& r)
  noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : msg_ctx_ptr_(msg_ctx_ptr), receiver_(new Receiver((Receiver2 &&) r)) {}

  void start() noexcept {
    try {
      msg_ctx_ptr_->client_.Invoke(
          msg_ctx_ptr_->func_name_, msg_ctx_ptr_->ctx_ptr_, msg_ctx_ptr_->req_, msg_ctx_ptr_->rsp_,
          [msg_ctx_ptr = msg_ctx_ptr_, receiver = receiver_](UnifexRpcStatus&& status) {
            unifex::set_value(std::move(*receiver), std::move(status), std::move(msg_ctx_ptr->rsp_));
          });
    } catch (...) {
      unifex::set_error(std::move(*receiver_), std::current_exception());
    }
  }

 private:
  std::shared_ptr<UnifexRpcMsgContext<Req, Rsp>> msg_ctx_ptr_;
  std::shared_ptr<Receiver> receiver_;
};

template <typename Req, typename Rsp>
class UnifexRpcSender {
 public:
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<UnifexRpcStatus, Rsp>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = false;

  UnifexRpcSender(const std::shared_ptr<UnifexRpcClient>& client_ptr, const std::string& func_name, const std::shared_ptr<const UnifexRpcContext>& ctx_ptr, const Req& req)
      : msg_ctx_ptr_(std::make_shared<UnifexRpcMsgContext<Req, Rsp>>(client_ptr, func_name, ctx_ptr, req)) {}

  template <typename Receiver>
  UnifexRpcOperationState<Req, Rsp, unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return UnifexRpcOperationState<Req, Rsp, unifex::remove_cvref_t<Receiver>>(msg_ctx_ptr_, (Receiver &&) receiver);
  }

 private:
  std::shared_ptr<UnifexRpcMsgContext<Req, Rsp>> msg_ctx_ptr_;
};

class UnifexRpcServiceProxy {
 protected:
  explicit UnifexRpcServiceProxy(const std::shared_ptr<UnifexRpcClient>& client_ptr)
      : client_ptr_(client_ptr) {}

  virtual ~UnifexRpcServiceProxy() {}

  template <typename Req, typename Rsp>
  auto Invoke(const std::string& func_name, const std::shared_ptr<const UnifexRpcContext>& ctx_ptr, const Req& req)
      -> UnifexRpcSender<Req, Rsp> {
    return UnifexRpcSender<Req, Rsp>(client_ptr_, func_name, ctx_ptr, req);
  }

 private:
  std::shared_ptr<UnifexRpcClient> client_ptr_;
};

}  // namespace ytrpc
}  // namespace ytlib