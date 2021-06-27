/**
 * @file log_svr.hpp
 * @brief 基于boost.asio的远程日志服务器
 * @details 基于boost.asio的远程日志服务器
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <set>

#include "net_util.hpp"
#include "ytlib/misc/error.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief 远程日志服务器
 * 默认监听50001端口，为每个ip-port创建一个文件夹存放滚动日志文件
 * 无协议，收到什么打印什么
 * 必须以智能指针形式构造，在
 */
class LogServer : public std::enable_shared_from_this<LogServer> {
 public:
  explicit LogServer(boost::asio::io_context& io, uint16_t port = 50001,
                     const std::filesystem::path& log_path = "./log",
                     std::ofstream::pos_type max_file_size = 1 * 1024 * 1024) : port_(port),
                                                                                io_(io),
                                                                                acceptor_(io, {IPV4(), port}),
                                                                                sessions_mgr_strand_(boost::asio::make_strand(io)),
                                                                                log_path_(log_path),
                                                                                max_file_size_(max_file_size) {}

  // 必须先主动调用Stop函数，等待所有协程结束，所有资源释放，所有智能指针释放，才能析构
  ~LogServer() {}

  // no copy
  LogServer(const LogServer&) = delete;
  LogServer& operator=(const LogServer&) = delete;

  void Start() {
    RT_ASSERT(CheckPort(port_), "port is used.");

    if (std::filesystem::status(log_path_).type() != std::filesystem::file_type::directory)
      std::filesystem::create_directories(log_path_);

    boost::asio::co_spawn(
        sessions_mgr_strand_,
        [this, auto self = shared_from_this()]() -> boost::asio::awaitable<void> {
          try {
            for (;;) {
              auto p_session = std::make_shared<LogSession>(
                  co_await acceptor_.async_accept(boost::asio::use_awaitable),
                  this);
              p_session->Start();

              session_ptr_set_.insert(p_session);
            }
          } catch (const std::exception& e) {
            std::cerr << "log svr accept connection get exception:" << e.what() << '\n';
          }

          acceptor_.close();
          acceptor_.release();

          co_return;
        },
        boost::asio::detached);
  }

  void Stop() {
    acceptor_.cancel();

    boost::asio::co_spawn(
        sessions_mgr_strand_,
        [this]() -> boost::asio::awaitable<void> {
          for (auto& session_ptr : session_ptr_set_)
            session_ptr->Stop();

          co_return;
        },
        boost::asio::detached);
  }

 private:
  class LogSession : public std::enable_shared_from_this<LogSession> {
   public:
    LogSession(TcpSocket&& sock, LogServer* logsvr_ptr) : sock_(std::move(sock)),
                                                          logsvr_ptr_(logsvr_ptr),
                                                          strand_(boost::asio::make_strand(logsvr_ptr->io_)),
                                                          timer_(strand_) {}

    // 必须先主动调用Stop函数，等待所有协程结束，所有资源释放，所有智能指针释放，才能析构
    ~LogSession() {}

    void Start() {
      const TcpEp& ep = sock_.remote_endpoint();
      DBG_PRINT("get a new connection from %s:%d", ep.address().to_string().c_str(), ep.port());

      log_file_dir_ = logsvr_ptr_->log_path_ / (ep.address().to_string() + "_" + std::to_string(ep.port()));

      if (std::filesystem::status(log_file_dir_).type() != std::filesystem::file_type::directory)
        std::filesystem::create_directories(log_file_dir_);

      DBG_PRINT("log session file dir: %s", log_file_dir_.string().c_str());

      auto self = shared_from_this();
      // 接收协程
      boost::asio::co_spawn(
          strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            try {
              const std::size_t data_size = 1024;
              char data[data_size];
              for (;;) {
                std::size_t n = co_await sock_.async_read_some(boost::asio::buffer(data, data_size), boost::asio::use_awaitable);

                time_t cnow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                struct tm now_tm;
#if defined(_WIN32)
                localtime_s(&now_tm, &cnow);
#else
                localtime_r(&cnow, &now_tm);
#endif

                if (!ofs_.is_open() || ofs_.tellp() > logsvr_ptr_->max_file_size_ ||
                    now_tm.tm_hour != t_.tm_hour || now_tm.tm_yday != t_.tm_yday) {
                  if (ofs_.is_open()) {
                    ofs_.flush();
                    ofs_.clear();
                    ofs_.close();
                  }

                  t_ = now_tm;
                  char name_buf[32];
                  snprintf(name_buf, sizeof(name_buf), "%04d%02d%02d_%02d%02d%02d.log",
                           t_.tm_year + 1900, t_.tm_mon + 1, t_.tm_mday, t_.tm_hour, t_.tm_min, t_.tm_sec);

                  ofs_.open(log_file_dir_ / name_buf, std::ios::app);
                }

                ofs_ << std::string_view(data, n);
                has_data = true;
              }
            } catch (std::exception& e) {
              std::cerr << "log session get exception and exit, addr:" << sock_.remote_endpoint() << ", exception:" << e.what() << '\n';
            }

            Stop();

            if (ofs_.is_open()) {
              ofs_.flush();
              ofs_.clear();
              ofs_.close();
            }

            sock_.close();
            sock_.release();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            try {
              const uint32_t dt = 5;                 // 定时器间隔，秒
              const uint32_t max_no_data_time = 60;  // 最长无数据时间，秒
              uint32_t no_data_time_count = 0;       // 无数据时间，秒

              for (;;) {
                timer_.expires_after(std::chrono::seconds(dt));
                co_await timer_.async_wait(boost::asio::use_awaitable);
                if (has_data && ofs_.is_open()) {
                  ofs_.flush();
                  has_data = false;
                  no_data_time_count = 0;
                } else {
                  no_data_time_count += dt;
                  if (no_data_time_count > max_no_data_time) {
                    break;
                  }
                }
              }
            } catch (const std::exception& e) {
              std::cerr << "log session timer get exception and exit, addr:" << sock_.remote_endpoint() << ", exception:" << e.what() << '\n';
            }

            Stop();

            co_return;
          },
          boost::asio::detached);
    }

    // 在stop的时候只是想办法结束各个协程，协程里面使用到的资源由各个协程自行释放。stop触发时机有2个：各个协程遇到异常时、上层主动调用
    void Stop() {
      sock_.cancel();
      timer_.cancel();

      auto self = shared_from_this();
      boost::asio::co_spawn(
          logsvr_ptr_->sessions_mgr_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            auto finditr = logsvr_ptr_->session_ptr_set_.find(self);
            if (finditr != logsvr_ptr_->session_ptr_set_.end())
              logsvr_ptr_->session_ptr_set_.erase(finditr);

            co_return;
          },
          boost::asio::detached);
    }

   private:
    TcpSocket sock_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;

    struct tm t_;                         // 当前文件创建时间
    std::ofstream ofs_;                   // 当前日志文件写入句柄
    std::filesystem::path log_file_dir_;  // 当前日志文件目录
    bool has_data = false;
    bool stopped_flag_ = false;
    LogServer* logsvr_ptr_;
  };

 private:
  const uint16_t port_;  //监听端口

  boost::asio::io_context& io_;
  boost::asio::ip::tcp::acceptor acceptor_;  //监听器

  boost::asio::strand<boost::asio::io_context::executor_type> sessions_mgr_strand_;  // session池操作strand
  std::set<std::shared_ptr<LogSession> > session_ptr_set_;                           // session池

  std::filesystem::path log_path_;         // 日志文件路径
  std::ofstream::pos_type max_file_size_;  // 日志文件最大大小
};

}  // namespace ytlib
