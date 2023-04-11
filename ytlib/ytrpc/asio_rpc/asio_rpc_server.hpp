/**
 * @file asio_rpc_server.hpp
 * @brief 基于boost.asio的RPC服务端
 * @note
 * @author WT
 * @date 2022-07-13
 */
#pragma once

#include <algorithm>
#include <concepts>
#include <list>
#include <memory>
#include <string>
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

class AsioRpcService {
 public:
  struct FuncAdapter {
    std::function<boost::asio::awaitable<AsioRpcStatus>(const std::shared_ptr<const AsioRpcContext>&, const google::protobuf::Message&, google::protobuf::Message&)> handle_func;
    std::function<std::unique_ptr<google::protobuf::Message>(void)> req_ptr_gener;
    std::function<std::unique_ptr<google::protobuf::Message>(void)> rsp_ptr_gener;
  };

  const std::unordered_map<std::string, FuncAdapter>& FuncAdapterMap() const {
    return func_adapter_map_;
  }

 protected:
  AsioRpcService() {}
  virtual ~AsioRpcService() {}

  template <typename ReqType, typename RspType>
  void RegisterRpcServiceFunc(
      const std::string& func_name,
      const std::function<boost::asio::awaitable<AsioRpcStatus>(const std::shared_ptr<const AsioRpcContext>&, const ReqType&, RspType&)>& func) {
    func_adapter_map_.emplace(
        func_name,
        FuncAdapter{
            .handle_func = [func](const std::shared_ptr<const AsioRpcContext>& ctx_ptr, const google::protobuf::Message& req, google::protobuf::Message& rsp) -> boost::asio::awaitable<AsioRpcStatus> {
              return func(ctx_ptr, static_cast<const ReqType&>(req), static_cast<RspType&>(rsp));
            },
            .req_ptr_gener = []() -> std::unique_ptr<google::protobuf::Message> {
              return std::make_unique<ReqType>();
            },
            .rsp_ptr_gener = []() -> std::unique_ptr<google::protobuf::Message> {
              return std::make_unique<RspType>();
            }});
  }

 private:
  std::unordered_map<std::string, FuncAdapter> func_adapter_map_;
};

class AsioRpcServer : public std::enable_shared_from_this<AsioRpcServer> {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    boost::asio::ip::tcp::endpoint ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4(), 55399};  // 监听的地址
    uint32_t max_session_num = 1000000;                                                                        // 最大连接数
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
   * @brief rpc server构造函数
   *
   * @param io_ptr
   * @param cfg
   */
  AsioRpcServer(const std::shared_ptr<boost::asio::io_context>& io_ptr, const AsioRpcServer::Cfg& cfg)
      : cfg_(AsioRpcServer::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioRpcServer::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        acceptor_(mgr_strand_, cfg_.ep),
        acceptor_timer_(mgr_strand_),
        mgr_timer_(mgr_strand_),
        func_map_ptr_(std::make_shared<std::unordered_map<std::string, AsioRpcService::FuncAdapter>>()) {}

  ~AsioRpcServer() {}

  AsioRpcServer(const AsioRpcServer&) = delete;             ///< no copy
  AsioRpcServer& operator=(const AsioRpcServer&) = delete;  ///< no copy

  template <std::derived_from<AsioRpcService> ServiceType>
  void RegisterService(const std::shared_ptr<ServiceType>& service_ptr) {
    const std::shared_ptr<const AsioRpcService>& rpc_service_ptr = std::static_pointer_cast<const AsioRpcService>(service_ptr);
    service_ptr_list_.emplace_back(rpc_service_ptr);
    func_map_ptr_->insert(rpc_service_ptr->FuncAdapterMap().begin(), rpc_service_ptr->FuncAdapterMap().end());

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

              auto session_ptr = std::make_shared<AsioRpcServer::Session>(io_ptr_, session_cfg_ptr_, func_map_ptr_);
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

  /**
   * @brief 获取配置
   *
   * @return const AsioRpcServer::Cfg&
   */
  const AsioRpcServer::Cfg& GetCfg() const { return cfg_; }

 private:
  // 包头结构：| 2byte magicnum | 2byte headlen | 4byte msglen |
  static const uint32_t HEAD_SIZE = 8;
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
            const std::shared_ptr<const AsioRpcServer::SessionCfg>& session_cfg_ptr,
            const std::shared_ptr<std::unordered_map<std::string, AsioRpcService::FuncAdapter>>& func_map_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          io_ptr_(io_ptr),
          session_socket_strand_(boost::asio::make_strand(*io_ptr)),
          sock_(session_socket_strand_),
          send_sig_timer_(session_socket_strand_),
          session_mgr_strand_(boost::asio::make_strand(*io_ptr)),
          timer_(session_mgr_strand_),
          func_map_ptr_(func_map_ptr) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void Start() {
      auto self = shared_from_this();

      // 发送协程
      boost::asio::co_spawn(
          session_socket_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_svr_session_send_co);

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
          session_socket_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_svr_session_recv_co);

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
                DBG_PRINT("rpc svr session async read %llu bytes", read_data_size);
                tick_has_data_ = true;

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

                  // msg长度为0表示是心跳包
                  if (pb_msg_len == 0) [[unlikely]] {
                    cur_handle_pos += HEAD_SIZE;
                    continue;
                  }

                  if ((HEAD_SIZE + pb_msg_len) > session_cfg_ptr_->max_recv_size) [[unlikely]]
                    throw std::runtime_error("Msg too large.");

                  // 如果HEAD_SIZE + pb_msg_len大于read_buf_size，则下次接收时buf会扩大
                  if (cur_unhandle_size < (HEAD_SIZE + pb_msg_len)) {
                    read_buf_size = std::max(read_buf_size, (HEAD_SIZE + pb_msg_len));
                    break;
                  }

                  const char* req_buf = read_buf_ptr.get() + cur_handle_pos + HEAD_SIZE;
                  const uint16_t pb_head_len = GetUint16FromBuf(read_buf_ptr.get() + cur_handle_pos + 2);

                  // 处理数据，需要post到整个io_ctx上
                  auto handle = [this, self, read_buf_ptr, req_buf, pb_head_len, pb_msg_len]() -> boost::asio::awaitable<void> {
                    try {
                      ReqHead req_head;
                      if (!req_head.ParseFromArray(req_buf, pb_head_len)) [[unlikely]]
                        throw std::runtime_error("Parse req head failed.");

                      BufferVec rsp_buf_vec;
                      BufferVecZeroCopyOutputStream os(rsp_buf_vec);
                      char* head_buf = static_cast<char*>(os.InitHead(AsioRpcServer::HEAD_SIZE));
                      head_buf[0] = AsioRpcServer::HEAD_BYTE_1;
                      head_buf[1] = AsioRpcServer::HEAD_BYTE_2;

                      RspHead rsp_head;
                      rsp_head.set_req_id(req_head.req_id());

                      std::unique_ptr<google::protobuf::Message> rsp_ptr;

                      // 查func
                      auto finditr = func_map_ptr_->find(req_head.func());
                      if (finditr != func_map_ptr_->end()) {
                        // 调用func
                        const AsioRpcService::FuncAdapter& func_adapter = finditr->second;
                        std::unique_ptr<google::protobuf::Message> req_ptr = func_adapter.req_ptr_gener();
                        if (!req_ptr->ParseFromArray(req_buf + pb_head_len, pb_msg_len - pb_head_len)) [[unlikely]] {
                          rsp_head.set_ret_code(static_cast<int32_t>(AsioRpcStatus::Code::SVR_PARSE_REQ_FAILED));
                        } else {
                          std::shared_ptr<AsioRpcContext> ctx_ptr = std::make_shared<AsioRpcContext>();
                          ctx_ptr->SetDeadline(std::chrono::system_clock::time_point(std::chrono::milliseconds(req_head.ddl_ms())));
                          ctx_ptr->ContextKv() = std::map<std::string, std::string>(req_head.context_kv().begin(), req_head.context_kv().end());

                          rsp_ptr = func_adapter.rsp_ptr_gener();
                          const AsioRpcStatus& ret_status = co_await func_adapter.handle_func(ctx_ptr, *req_ptr, *rsp_ptr);
                          rsp_head.set_ret_code(static_cast<int32_t>(ret_status.Ret()));
                          rsp_head.set_func_ret_code(static_cast<int32_t>(ret_status.FuncRet()));
                          rsp_head.set_func_ret_msg(ret_status.FuncRetMsg());
                        }
                      } else {
                        rsp_head.set_ret_code(static_cast<int32_t>(AsioRpcStatus::Code::NOT_FOUND));
                      }

                      if (!rsp_head.SerializeToZeroCopyStream(&os)) [[unlikely]]
                        throw std::runtime_error("Serialize rsp head failed.");
                      SetBufFromUint16(&head_buf[2], static_cast<uint16_t>(os.ByteCount() - AsioRpcServer::HEAD_SIZE));

                      if (rsp_ptr) {
                        if (!rsp_ptr->SerializeToZeroCopyStream(&os)) [[unlikely]]
                          throw std::runtime_error("Serialize rsp failed.");
                      }
                      SetBufFromUint32(&head_buf[4], static_cast<uint32_t>(os.ByteCount() - AsioRpcServer::HEAD_SIZE));

                      rsp_buf_vec.CommitLastBuf(os.LastBufSize());

                      boost::asio::dispatch(
                          session_socket_strand_,
                          [this, self, rsp_buf_vec{std::move(rsp_buf_vec)}]() mutable {
                            send_buffer_vec_.Merge(rsp_buf_vec);
                            send_sig_timer_.cancel();
                          });

                    } catch (const std::exception& e) {
                      DBG_PRINT("rpc svr session handle data co get exception and exit, exception info: %s", e.what());
                    }

                    co_return;
                  };

                  boost::asio::post(
                      *io_ptr_,
                      [this, self, handle{std::move(handle)}]() mutable {
                        boost::asio::co_spawn(
                            *io_ptr_,
                            std::move(handle),
                            boost::asio::detached);
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
              DBG_PRINT("rpc svr session recv co get exception and exit, exception info: %s", e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          session_mgr_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(rpc_svr_session_timer_co);

            try {
              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->max_no_data_duration);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                } else {
                  DBG_PRINT("rpc svr session exit due to timeout(%llums), addr %s.",
                            std::chrono::duration_cast<std::chrono::milliseconds>(session_cfg_ptr_->max_no_data_duration).count(),
                            TcpEp2Str(sock_.remote_endpoint()).c_str());
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
          session_socket_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(rpc_svr_session_sock_stop_co);

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
                DBG_PRINT("rpc svr session sock stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });

      boost::asio::dispatch(
          session_mgr_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(rpc_svr_session_mgr_stop_co);

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
                DBG_PRINT("rpc svr session mgr stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    boost::asio::ip::tcp::socket& Socket() { return sock_; }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const AsioRpcServer::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    std::shared_ptr<boost::asio::io_context> io_ptr_;

    boost::asio::strand<boost::asio::io_context::executor_type> session_socket_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer send_sig_timer_;

    boost::asio::strand<boost::asio::io_context::executor_type> session_mgr_strand_;
    boost::asio::steady_timer timer_;

    std::atomic_bool tick_has_data_ = false;
    BufferVec send_buffer_vec_;

    const std::shared_ptr<const std::unordered_map<std::string, AsioRpcService::FuncAdapter>> func_map_ptr_;
  };

 private:
  const AsioRpcServer::Cfg cfg_;
  std::atomic_bool start_flag_ = false;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const AsioRpcServer::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;  // session池操作strand
  boost::asio::ip::tcp::acceptor acceptor_;                                 // 监听器
  boost::asio::steady_timer acceptor_timer_;                                // 连接满时监听器的sleep定时器
  boost::asio::steady_timer mgr_timer_;                                     // 管理session池的定时器
  std::list<std::shared_ptr<AsioRpcServer::Session>> session_ptr_list_;     // session池

  std::list<std::shared_ptr<const AsioRpcService>> service_ptr_list_;
  std::shared_ptr<std::unordered_map<std::string, AsioRpcService::FuncAdapter>> func_map_ptr_;
};

}  // namespace ytrpc
}  // namespace ytlib
