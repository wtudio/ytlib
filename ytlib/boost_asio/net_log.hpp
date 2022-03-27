/**
 * @file net_log.hpp
 * @brief 基于boost.asio的远程日志服务器
 * @note 基于boost.asio的远程日志服务器
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

namespace ytlib {

/**
 * @brief 远程日志服务器客户端
 * @note 必须以智能指针形式构造，在结束使用前手动调用Stop方法
 */
class NetLogClient : public std::enable_shared_from_this<NetLogClient> {
 public:
  /**
   * @brief 远程日志服务器客户端构造函数
   *
   * @param io_ptr io_context智能指针
   * @param log_svr_ep 日志服务器地址
   */
  NetLogClient(std::shared_ptr<boost::asio::io_context> io_ptr, const TcpEp& log_svr_ep)
      : log_svr_ep_(log_svr_ep),
        io_ptr_(io_ptr),
        strand_(boost::asio::make_strand(*io_ptr_)),
        sock_(strand_) {}

  ~NetLogClient() {}

  NetLogClient(const NetLogClient&) = delete;             ///< no copy
  NetLogClient& operator=(const NetLogClient&) = delete;  ///< no copy

  /**
   * @brief 停止
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (std::atomic_exchange(&stop_flag_, true)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(net_log_cli_stop_co);

          if (sock_.is_open()) {
            sock_.cancel();
            sock_.close();
            sock_.release();
          }
        });
  }

  /**
   * @brief 打日志到远程日志服务器
   *
   * @param log_buf_ptr 日志内容指针
   */
  void LogToSvr(std::shared_ptr<boost::asio::streambuf> log_buf_ptr) {
    if (stop_flag_) [[unlikely]]
      return;

    auto self = shared_from_this();
    boost::asio::co_spawn(
        strand_,
        [this, self, log_buf_ptr]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(net_log_cli_log_to_svr_co);

          try {
            if (!sock_.is_open()) {
              DBG_PRINT("start create a new connect to %s", TcpEp2Str(log_svr_ep_).c_str());
              co_await sock_.async_connect(log_svr_ep_, boost::asio::use_awaitable);
            }

            while (log_buf_ptr->size()) {
              size_t n = co_await sock_.async_write_some(log_buf_ptr->data(), boost::asio::use_awaitable);
              log_buf_ptr->consume(n);
            }

          } catch (const std::exception& e) {
            DBG_PRINT("send log to svr get exception, addr %s, exception %s", TcpEp2Str(log_svr_ep_).c_str(), e.what());
            if (sock_.is_open()) {
              sock_.cancel();
              sock_.close();
              sock_.release();
            }
          }

          co_return;
        },
        boost::asio::detached);
  }

 private:
  const TcpEp log_svr_ep_;
  std::atomic_bool stop_flag_ = false;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  TcpSocket sock_;
};

/**
 * @brief 远程日志服务器配置
 */
struct LogSvrCfg {
  LogSvrCfg() : log_path("./log") {}

  uint16_t port = 50001;                   // 监听的端口
  std::filesystem::path log_path;          // 日志路径
  size_t max_file_size = 1 * 1024 * 1024;  // 最大日志文件尺寸

  uint32_t timer_dt = 5;           // 定时器间隔，秒
  uint32_t max_no_data_time = 60;  // 最长无数据时间，秒
};

/**
 * @brief 远程日志服务器
 * @note 默认监听50001端口，为每个ip-port创建一个文件夹存放滚动日志文件
 * 无协议，收到什么打印什么
 * 必须以智能指针形式构造，调用Start启动服务，在结束使用前手动调用Stop方法
 */
class LogSvr : public std::enable_shared_from_this<LogSvr> {
 public:
  /**
   * @brief 远程日志服务器构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  LogSvr(std::shared_ptr<boost::asio::io_context> io_ptr, const LogSvrCfg& cfg)
      : cfg_(cfg),
        io_ptr_(io_ptr),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {
    // 校验配置
    if (cfg_.port > 65535 || cfg_.port < 1000) cfg_.port = 50001;

    if (cfg_.max_file_size < 100 * 1024) cfg_.max_file_size = 100 * 1024;
    if (cfg_.max_file_size > 1024 * 1024 * 1024) cfg_.max_file_size = 1024 * 1024 * 1024;
  }

  ~LogSvr() {}

  LogSvr(const LogSvr&) = delete;             ///< no copy
  LogSvr& operator=(const LogSvr&) = delete;  ///< no copy

  /**
   * @brief 启动日志服务器
   *
   */
  void Start() {
    acceptor_ptr_ = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_ptr_, TcpEp{IPV4(), cfg_.port});

    if (std::filesystem::status(cfg_.log_path).type() != std::filesystem::file_type::directory)
      std::filesystem::create_directories(cfg_.log_path);

    auto self = shared_from_this();
    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(net_log_svr_acceptor_co);
          try {
            while (run_flag_) {
              auto session_ptr = std::make_shared<LogSession>(
                  co_await acceptor_ptr_->async_accept(boost::asio::use_awaitable),
                  self);
              session_ptr->Start();

              session_ptr_set_.emplace(session_ptr);
            }
          } catch (const std::exception& e) {
            DBG_PRINT("accept connection get exception %s", e.what());
          }

          Stop();

          co_return;
        },
        boost::asio::detached);
  }

  /**
   * @brief 停止日志服务器
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(net_log_svr_stop_co);

          if (acceptor_ptr_->is_open()) {
            acceptor_ptr_->cancel();
            acceptor_ptr_->close();
            acceptor_ptr_->release();
          }

          while (!session_ptr_set_.empty()) {
            auto session_ptr_itr = session_ptr_set_.begin();
            auto session_ptr = *session_ptr_itr;
            session_ptr_set_.erase(session_ptr_itr);

            session_ptr->Stop();
          }
        });
  }

 private:
  class LogSession : public std::enable_shared_from_this<LogSession> {
   public:
    LogSession(TcpSocket&& sock, std::shared_ptr<LogSvr> logsvr_ptr)
        : logsvr_ptr_(logsvr_ptr),
          strand_(boost::asio::make_strand(*(logsvr_ptr_->io_ptr_))),
          sock_(std::move(sock)),
          timer_(strand_) {
      const TcpEp& ep = sock_.remote_endpoint();
      session_name_ = (ep.address().to_string() + "_" + std::to_string(ep.port()));
    }

    ~LogSession() {}

    LogSession(const LogSession&) = delete;             ///< no copy
    LogSession& operator=(const LogSession&) = delete;  ///< no copy

    void Start() {
      std::filesystem::path log_file_dir = logsvr_ptr_->cfg_.log_path / session_name_;

      if (std::filesystem::status(log_file_dir).type() != std::filesystem::file_type::directory)
        std::filesystem::create_directories(log_file_dir);

      log_file_ = log_file_dir / (session_name_ + ".log");

      DBG_PRINT("get a new connection from %s, log session file dir: %s", session_name_.c_str(), log_file_dir.string().c_str());

      auto self = shared_from_this();
      // 接收协程
      boost::asio::co_spawn(
          strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(net_log_svr_session_recv_co);
            try {
              const size_t data_size = 1024;
              char data[data_size];
              while (run_flag_) {
                size_t n = co_await sock_.async_read_some(boost::asio::buffer(data, data_size), boost::asio::use_awaitable);

                if (!ofs_.is_open())
                  ofs_.open(log_file_, std::ios::app);

                ofs_ << std::string_view(data, n);
                tick_has_data_ = true;
              }
            } catch (std::exception& e) {
              DBG_PRINT("log session get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(net_log_svr_session_timer_co);
            try {
              uint32_t no_data_time_count = 0;  // 当前无数据时间，秒

              while (run_flag_) {
                timer_.expires_after(std::chrono::seconds(logsvr_ptr_->cfg_.timer_dt));
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                  no_data_time_count = 0;

                  if (ofs_.is_open()) ofs_.flush();

                  // 判断日志文件是否需要rename
                  if (std::filesystem::file_size(log_file_) > logsvr_ptr_->cfg_.max_file_size) {
                    if (ofs_.is_open()) {
                      ofs_.clear();
                      ofs_.close();
                    }
                    std::filesystem::rename(log_file_, log_file_.string() + "_" + GetCurTimeStr());
                  }

                } else {
                  no_data_time_count += logsvr_ptr_->cfg_.timer_dt;
                  if (no_data_time_count >= logsvr_ptr_->cfg_.max_no_data_time) {
                    DBG_PRINT("log session exit due to timeout(%us), addr %s.", no_data_time_count, session_name_.c_str());
                    break;
                  }
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("log session timer get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
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
          strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(net_log_svr_session_stop_co);

            if (sock_.is_open()) {
              sock_.cancel();
              sock_.close();
              sock_.release();
            }

            timer_.cancel();

            if (ofs_.is_open()) {
              ofs_.flush();
              ofs_.clear();
              ofs_.close();
            }
          });

      boost::asio::dispatch(
          logsvr_ptr_->mgr_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(net_log_svr_session_clear_co);

            auto finditr = logsvr_ptr_->session_ptr_set_.find(self);
            if (finditr != logsvr_ptr_->session_ptr_set_.end())
              logsvr_ptr_->session_ptr_set_.erase(finditr);
          });
    }

   private:
    std::atomic_bool run_flag_ = true;
    std::shared_ptr<LogSvr> logsvr_ptr_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    TcpSocket sock_;
    boost::asio::steady_timer timer_;

    std::string session_name_;
    std::filesystem::path log_file_;  // 当前日志文件路径
    std::ofstream ofs_;               // 当前日志文件写入句柄

    bool tick_has_data_ = false;
  };

 private:
  std::atomic_bool run_flag_ = true;
  LogSvrCfg cfg_;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;  // session池操作strand
  std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr_;            // 监听器
  std::set<std::shared_ptr<LogSession> > session_ptr_set_;                  // session池
};

}  // namespace ytlib
