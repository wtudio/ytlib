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

#include "net_util.hpp"
#include "ytlib/misc/error.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief 远程日志服务器
 * 默认监听50001端口，为每个ip-port创建一个文件夹存放滚动日志文件
 * 无协议，收到什么打印什么
 */
class LogServer {
 public:
  // 日志相关配置
  struct LogConfig {
    std::filesystem::path log_path;
    std::ofstream::pos_type max_file_size;
  };
  using LogCfgPtr = std::shared_ptr<LogConfig>;

 public:
  explicit LogServer(boost::asio::io_context& io, uint16_t port = 50001,
                     LogCfgPtr log_cfg_ptr = LogCfgPtr()) : port_(port),
                                                            io_(io),
                                                            acceptor_(io, {IPV4(), port}),
                                                            sessions_strand_(boost::asio::make_strand(io)),
                                                            log_cfg_ptr_(log_cfg_ptr) {
    if (!log_cfg_ptr_) {
      log_cfg_ptr_ = std::make_shared<LogConfig>();
      log_cfg_ptr_->log_path = "./log";
      log_cfg_ptr_->max_file_size = 1 * 1024 * 1024;
    }
    log_cfg_ptr_->log_path = std::filesystem::absolute(log_cfg_ptr_->log_path);
  }
  ~LogServer() {}

  // no copy
  LogServer(const LogServer&) = delete;
  LogServer& operator=(const LogServer&) = delete;

  void Start() {
    RT_ASSERT(CheckPort(port_), "port is used.");

    if (std::filesystem::status(log_cfg_ptr_->log_path).type() != std::filesystem::file_type::directory)
      std::filesystem::create_directories(log_cfg_ptr_->log_path);

    boost::asio::co_spawn(
        io_,
        [this]() -> boost::asio::awaitable<void> {
          try {
            for (;;) {
              auto p_session = std::make_shared<LogSession>(
                  co_await acceptor_.async_accept(boost::asio::use_awaitable),
                  io_,
                  log_cfg_ptr_);

              boost::asio::co_spawn(
                  sessions_strand_,
                  [this, p_session]() -> boost::asio::awaitable<void> {
                    sessions_.emplace(sessions_.end(), p_session);
                  },
                  boost::asio::detached);

              p_session->Start();
            }
          } catch (const std::exception& e) {
            std::cerr << "log svr accept connection get exception:" << e.what() << '\n';
          }
          co_return;
        },
        boost::asio::detached);
  }

  void Stop() {
  }

 private:
  class LogSession : public std::enable_shared_from_this<LogSession> {
   public:
    LogSession(TcpSocket&& sock, boost::asio::io_context& io,
               LogCfgPtr log_cfg_ptr) : sock_(std::move(sock)),
                                        strand_(boost::asio::make_strand(io)),
                                        log_cfg_ptr_(log_cfg_ptr) {}

    ~LogSession() {
      if (sock_.is_open()) {
        sock_.close();
      }

      if (ofs_.is_open()) {
        ofs_.flush();
        ofs_.clear();
        ofs_.close();
      }
    }

    void Start() {
      const TcpEp& ep = sock_.remote_endpoint();
      DBG_PRINT("get a new connection from %s:%d", ep.address().to_string().c_str(), ep.port());

      log_file_dir_ = log_cfg_ptr_->log_path / (ep.address().to_string() + "_" + std::to_string(ep.port()));

      if (std::filesystem::status(log_file_dir_).type() != std::filesystem::file_type::directory)
        std::filesystem::create_directories(log_file_dir_);

      DBG_PRINT("log session file dir: %s", log_file_dir_.string().c_str());

      auto self = shared_from_this();
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

                if (!ofs_.is_open() || ofs_.tellp() > log_cfg_ptr_->max_file_size ||
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
              }
            } catch (std::exception& e) {
              std::cerr << "log session get exception and exit, addr:" << sock_.remote_endpoint() << ", exception:" << e.what() << '\n';
            }
            co_return;
          },
          boost::asio::detached);

      boost::asio::co_spawn(
          strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            try {
              boost::asio::steady_timer t(strand_);
              for (;;) {
                t.expires_after(std::chrono::seconds(5));
                co_await t.async_wait(boost::asio::use_awaitable);
                if (ofs_.is_open()) {
                  ofs_.flush();
                }
              }
            } catch (const std::exception& e) {
              std::cerr << "log session timer get exception and exit, addr:" << sock_.remote_endpoint() << ", exception:" << e.what() << '\n';
            }
            co_return;
          },
          boost::asio::detached);
    }

   private:
    TcpSocket sock_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    struct tm t_;                         // 当前文件创建时间
    std::ofstream ofs_;                   // 日志文件
    std::filesystem::path log_file_dir_;  // 日志文件目录

    LogCfgPtr log_cfg_ptr_;  // 日志配置
  };

 private:
  const uint16_t port_;  //监听端口

  boost::asio::io_context& io_;
  boost::asio::ip::tcp::acceptor acceptor_;  //监听器

  boost::asio::strand<boost::asio::io_context::executor_type> sessions_strand_;  // session池操作strand
  std::list<LogSession> sessions_;                                               // session池

  LogCfgPtr log_cfg_ptr_;  // 日志配置
};

}  // namespace ytlib
