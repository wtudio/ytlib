/**
 * @file asio_net_log_svr.hpp
 * @brief 基于boost.asio的远程日志服务器
 * @note 基于boost.asio的远程日志服务器
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

namespace ytlib {

/**
 * @brief 远程日志服务器
 * @note 默认监听52684端口，为每个ip-port创建一个文件夹存放滚动日志文件
 * 无协议，收到什么打印什么
 * 必须以智能指针形式构造，调用Start启动服务，在结束使用前手动调用Stop方法
 * todo: 同地址多个连接时的处理
 */
class AsioNetLogServer : public std::enable_shared_from_this<AsioNetLogServer> {
 public:
  /**
   * @brief 远程日志服务器配置
   */
  struct Cfg {
    uint16_t port = 52684;                     // 监听的端口
    std::filesystem::path log_path = "./log";  // 日志路径
    size_t max_file_size = 1 * 1024 * 1024;    // 最大日志文件尺寸

    size_t max_session_num = 1000000;                                            // 最大连接数
    std::chrono::steady_clock::duration mgr_timer_dt = std::chrono::seconds(5);  // 管理协程定时器间隔

    size_t session_buf_size = 1024 * 16;                                                  // session接受数据时的缓存
    std::chrono::steady_clock::duration timer_dt = std::chrono::seconds(5);               // 定时器间隔
    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(60);  // 最长无数据时间

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.port > 65535 || cfg.port < 1000) cfg.port = 52684;

      if (cfg.max_file_size < 100 * 1024) cfg.max_file_size = 100 * 1024;
      if (cfg.max_file_size > 1024 * 1024 * 1024) cfg.max_file_size = 1024 * 1024 * 1024;

      if (cfg.max_session_num < 1) cfg.max_session_num = 1;
      if (cfg.max_session_num > boost::asio::ip::tcp::acceptor::max_listen_connections)
        cfg.max_session_num = boost::asio::ip::tcp::acceptor::max_listen_connections;
      if (cfg.mgr_timer_dt < std::chrono::milliseconds(100)) cfg.mgr_timer_dt = std::chrono::milliseconds(100);

      if (cfg.timer_dt < std::chrono::milliseconds(100)) cfg.timer_dt = std::chrono::milliseconds(100);
      if (cfg.max_no_data_duration < cfg.timer_dt * 2) cfg.max_no_data_duration = cfg.timer_dt * 2;

      return cfg;
    }
  };

  /**
   * @brief 远程日志服务器构造函数
   *
   * @param io_ptr io_context智能指针
   * @param cfg 配置
   */
  AsioNetLogServer(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioNetLogServer::Cfg& cfg)
      : cfg_(AsioNetLogServer::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioNetLogServer::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        acceptor_(mgr_strand_, boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4(), cfg_.port}),
        acceptor_timer_(mgr_strand_),
        mgr_timer_(mgr_strand_) {}

  ~AsioNetLogServer() {}

  AsioNetLogServer(const AsioNetLogServer&) = delete;             ///< no copy
  AsioNetLogServer& operator=(const AsioNetLogServer&) = delete;  ///< no copy

  /**
   * @brief 启动日志服务器
   *
   */
  void Start() {
    if (std::filesystem::status(cfg_.log_path).type() != std::filesystem::file_type::directory)
      std::filesystem::create_directories(cfg_.log_path);

    auto self = shared_from_this();
    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(net_log_svr_acceptor_co);

          while (run_flag_) {
            try {
              // 如果链接数达到上限，则等待一段时间再试
              if (session_ptr_list_.size() >= cfg_.max_session_num) {
                acceptor_timer_.expires_after(cfg_.mgr_timer_dt);
                co_await acceptor_timer_.async_wait(boost::asio::use_awaitable);
                continue;
              }

              auto session_ptr = std::make_shared<AsioNetLogServer::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_);
              co_await acceptor_.async_accept(session_ptr->Socket(), boost::asio::use_awaitable);
              session_ptr->Start();

              session_ptr_list_.emplace_back(session_ptr);

            } catch (const std::exception& e) {
              DBG_PRINT("net log svr accept connection get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);

    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(net_log_svr_timer_co);
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
              DBG_PRINT("net log svr timer get exception and exit, exception info: %s", e.what());
            }
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
              DBG_PRINT("net log svr stop get exception at step %u, exception info: %s", stop_step, e.what());
              ++stop_step;
            }
          }

          for (auto& session_ptr : session_ptr_list_)
            session_ptr->Stop();

          session_ptr_list_.clear();
        });
  }

 private:
  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : log_path(cfg.log_path),
          max_file_size(cfg.max_file_size),
          session_buf_size(cfg.session_buf_size),
          timer_dt(cfg.timer_dt),
          max_no_data_duration(cfg.max_no_data_duration) {}

    std::filesystem::path log_path;
    size_t max_file_size;
    size_t session_buf_size;
    std::chrono::steady_clock::duration timer_dt;
    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(boost::asio::strand<boost::asio::io_context::executor_type> session_strand,
            std::shared_ptr<const AsioNetLogServer::SessionCfg> session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          sock_(session_strand_),
          timer_(session_strand_) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void Start() {
      const boost::asio::ip::tcp::endpoint& ep = sock_.remote_endpoint();
      session_name_ = (ep.address().to_string() + "_" + std::to_string(ep.port()));

      std::filesystem::path log_file_dir = session_cfg_ptr_->log_path / session_name_;

      if (std::filesystem::status(log_file_dir).type() != std::filesystem::file_type::directory)
        std::filesystem::create_directories(log_file_dir);

      log_file_ = log_file_dir / (session_name_ + ".log");

      DBG_PRINT("net log svr get a new connection from %s, log session file dir: %s", session_name_.c_str(), log_file_dir.string().c_str());

      auto self = shared_from_this();

      // 接收协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(net_log_svr_session_recv_co);
            try {
              std::vector<char> session_buf(session_cfg_ptr_->session_buf_size);
              while (run_flag_) {
                size_t read_data_size = co_await sock_.async_read_some(boost::asio::buffer(session_buf), boost::asio::use_awaitable);
                DBG_PRINT("net log svr session async read %llu bytes", read_data_size);

                if (!ofs_.is_open())
                  ofs_.open(log_file_, std::ios::app);

                ofs_ << std::string_view(session_buf.data(), read_data_size);
                tick_has_data_ = true;
              }
            } catch (std::exception& e) {
              DBG_PRINT("net log svr session get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(net_log_svr_session_timer_co);

            try {
              namespace chrono = std::chrono;
              chrono::steady_clock::time_point last_data_time_point = chrono::steady_clock::now();  // 上次有数据时间

              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->timer_dt);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                  last_data_time_point = chrono::steady_clock::now();

                  if (ofs_.is_open()) ofs_.flush();

                  // 判断日志文件是否需要rename
                  if (std::filesystem::file_size(log_file_) > session_cfg_ptr_->max_file_size) {
                    if (ofs_.is_open()) {
                      ofs_.clear();
                      ofs_.close();
                    }
                    std::filesystem::rename(log_file_, log_file_.string() + "_" + GetCurTimeStr());
                  }

                } else {
                  chrono::steady_clock::duration no_data_duration = chrono::steady_clock::now() - last_data_time_point;
                  if (no_data_duration >= session_cfg_ptr_->max_no_data_duration) {
                    DBG_PRINT("net log svr session exit due to timeout(%llums), addr %s.", chrono::duration_cast<chrono::milliseconds>(no_data_duration).count(), session_name_.c_str());
                    break;
                  }
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("net log svr session timer get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
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
            ASIO_DEBUG_HANDLE(net_log_svr_session_stop_co);

            uint32_t stop_step = 1;
            while (stop_step) {
              try {
                switch (stop_step) {
                  case 1:
                    timer_.cancel();
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
                  case 6:
                    if (ofs_.is_open()) {
                      ofs_.flush();
                      ofs_.clear();
                      ofs_.close();
                    }
                    ++stop_step;
                  default:
                    stop_step = 0;
                    break;
                }
              } catch (const std::exception& e) {
                DBG_PRINT("net log svr session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    boost::asio::ip::tcp::socket& Socket() { return sock_; }

    const std::atomic_bool& IsRunning() { return run_flag_; }

   private:
    std::shared_ptr<const AsioNetLogServer::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::asio::ip::tcp::socket sock_;
    boost::asio::steady_timer timer_;

    std::string session_name_;
    std::filesystem::path log_file_;  // 当前日志文件路径
    std::ofstream ofs_;               // 当前日志文件写入句柄
    bool tick_has_data_ = false;
  };

 private:
  const AsioNetLogServer::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const AsioNetLogServer::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;   // session池操作strand
  boost::asio::ip::tcp::acceptor acceptor_;                                  // 监听器
  boost::asio::steady_timer acceptor_timer_;                                 // 连接满时监听器的sleep定时器
  boost::asio::steady_timer mgr_timer_;                                      // 管理session池的定时器
  std::list<std::shared_ptr<AsioNetLogServer::Session> > session_ptr_list_;  // session池
};

}  // namespace ytlib
