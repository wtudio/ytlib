/**
 * @file asio_http_cli.hpp
 * @brief 基于boost.beast的http客户端
 * @note 基于boost.beast的http客户端
 * @author WT
 * @date 2022-04-16
 */
#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief http客户端
 *
 */
class AsioHttpClientProxy : public std::enable_shared_from_this<AsioHttpClientProxy> {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    std::string host;     // 服务器域名或ip
    std::string service;  // 服务（如http、ftp）或端口号

    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(60);  // 连接最长无数据时间

    size_t max_session_num = 1000;  // 最大连接数

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.max_session_num < 1) cfg.max_session_num = 1;

      return cfg;
    }
  };

  /**
   * @brief http client proxy构造函数
   *
   * @param io_ptr io_context
   * @param cfg 配置
   */
  AsioHttpClientProxy(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioHttpClientProxy::Cfg& cfg)
      : cfg_(AsioHttpClientProxy::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioHttpClientProxy::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioHttpClientProxy() {}

  AsioHttpClientProxy(const AsioHttpClientProxy&) = delete;             ///< no copy
  AsioHttpClientProxy& operator=(const AsioHttpClientProxy&) = delete;  ///< no copy

  /**
   * @brief http请求协程接口
   *
   * @tparam ReqBodyType 请求包的body类型
   * @tparam RspBodyType 返回包的body类型
   * @param req 请求包
   * @param timeout 超时时间
   * @return boost::asio::awaitable<boost::beast::http::response<RspBodyType> > 返回包协程句柄
   */
  template <typename ReqBodyType = boost::beast::http::string_body, typename RspBodyType = boost::beast::http::string_body>
  boost::asio::awaitable<boost::beast::http::response<RspBodyType> > HttpSendRecvCo(const boost::beast::http::request<ReqBodyType>& req,
                                                                                    std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
    if (!run_flag_) [[unlikely]]
      throw std::runtime_error("Http client proxy is closed.");

    // 找可用session，没有就新建一个。同时清理已失效session
    std::shared_ptr<AsioHttpClientProxy::Session> session_ptr;
    for (auto itr = session_ptr_list_.begin(); itr != session_ptr_list_.end();) {
      if ((*itr)->IsRunning()) {
        if ((*itr)->CheckIdleAndUse()) {
          session_ptr = *itr;
          break;
        }
        ++itr;
      } else {
        session_ptr_list_.erase(itr++);
      }
    }

    if (!session_ptr) {
      if (session_ptr_list_.size() >= cfg_.max_session_num)
        throw std::runtime_error("Http client session num reach the upper limit.");

      session_ptr = std::make_shared<AsioHttpClientProxy::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_);
      session_ptr->Start();
      session_ptr_list_.emplace_back(session_ptr);
    }

    co_return co_await boost::asio::co_spawn(
        session_ptr->Strand(),
        session_ptr->HttpSendRecvCo(req, timeout),
        boost::asio::use_awaitable);
  }

  /**
   * @brief 停止
   * @note 在析构之前需要手动调用此函数
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(http_cli_proxy_stop_co);

          for (auto& session_ptr : session_ptr_list_)
            session_ptr->Stop();

          session_ptr_list_.clear();
        });
  }

  /**
   * @brief 是否在运行
   *
   * @return const std::atomic_bool&
   */
  const std::atomic_bool& IsRunning() { return run_flag_; }

  /**
   * @brief 调用HttpSendRecvCo时的strand
   *
   * @return boost::asio::strand<boost::asio::io_context::executor_type>&
   */
  boost::asio::strand<boost::asio::io_context::executor_type>& Strand() {
    return mgr_strand_;
  }

 private:
  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : host(cfg.host),
          service(cfg.service),
          max_no_data_duration(cfg.max_no_data_duration) {}

    std::string host;
    std::string service;
    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(boost::asio::strand<boost::asio::io_context::executor_type> session_strand,
            std::shared_ptr<const AsioHttpClientProxy::SessionCfg> session_cfg_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          stream_(session_strand_),
          timer_(session_strand_) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    template <typename ReqBodyType = boost::beast::http::string_body, typename RspBodyType = boost::beast::http::string_body>
    boost::asio::awaitable<boost::beast::http::response<RspBodyType> > HttpSendRecvCo(const boost::beast::http::request<ReqBodyType>& req,
                                                                                      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
      if (!run_flag_) [[unlikely]]
        throw std::logic_error("http client session is closed.");

      try {
        namespace chrono = std::chrono;
        namespace asio = boost::asio;
        namespace http = boost::beast::http;

        chrono::steady_clock::time_point start_time_point = chrono::steady_clock::now();
        chrono::steady_clock::duration cur_duration;

        if (first_time_entry_) [[unlikely]] {
          first_time_entry_ = false;

          // resolve
          asio::ip::tcp::resolver resolver(session_strand_);
          auto const dst = co_await resolver.async_resolve(session_cfg_ptr_->host, session_cfg_ptr_->service, asio::use_awaitable);

          // connect
          cur_duration = chrono::steady_clock::now() - start_time_point;
          if (cur_duration >= timeout) [[unlikely]]
            throw std::logic_error("Timeout.");

          DBG_PRINT("http cli session async connect, timeout %llums", chrono::duration_cast<chrono::milliseconds>(timeout - cur_duration).count());
          stream_.expires_after(timeout - cur_duration);
          co_await stream_.async_connect(dst, asio::use_awaitable);
        }

        // write
        cur_duration = chrono::steady_clock::now() - start_time_point;
        if (cur_duration >= timeout) [[unlikely]]
          throw std::logic_error("Timeout.");

        DBG_PRINT("http cli session async write, timeout %llums", chrono::duration_cast<chrono::milliseconds>(timeout - cur_duration).count());
        stream_.expires_after(timeout - cur_duration);
        size_t write_size = co_await http::async_write(stream_, req, asio::use_awaitable);
        DBG_PRINT("http cli session write %llu bytes", write_size);

        // read
        cur_duration = chrono::steady_clock::now() - start_time_point;
        if (cur_duration >= timeout) [[unlikely]]
          throw std::logic_error("Timeout.");

        DBG_PRINT("http cli session async read, timeout %llums", chrono::duration_cast<chrono::milliseconds>(timeout - cur_duration).count());
        stream_.expires_after(timeout - cur_duration);
        boost::beast::http::response<RspBodyType> rsp;
        size_t read_size = co_await http::async_read(stream_, buffer_, rsp, asio::use_awaitable);
        DBG_PRINT("http cli session read %llu bytes", read_size);

        if (req.need_eof() || rsp.need_eof()) {
          DBG_PRINT("http cli session close due to eof");
          Stop();
        }

        idle_flag_ = true;

        co_return rsp;

      } catch (const std::exception& e) {
        DBG_PRINT("http cli session send recv co get exception and exit, exception info: %s", e.what());
      }

      Stop();

      throw std::logic_error("http client session send & recv failed and exit.");
    }

    void Start() {
      auto self = shared_from_this();

      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(http_cli_session_timer_co);

            try {
              namespace chrono = std::chrono;
              chrono::steady_clock::time_point last_data_time_point = chrono::steady_clock::now();  // 上次有数据时间

              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->max_no_data_duration);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                  last_data_time_point = chrono::steady_clock::now();
                } else {
                  chrono::steady_clock::duration no_data_duration = chrono::steady_clock::now() - last_data_time_point;
                  if (no_data_duration >= session_cfg_ptr_->max_no_data_duration) {
                    DBG_PRINT("http cli session exit due to timeout(%llums).", chrono::duration_cast<chrono::milliseconds>(no_data_duration).count());
                    break;
                  }
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("http cli session timer get exception and exit, exception info: %s", e.what());
            }

            Stop();

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
            ASIO_DEBUG_HANDLE(http_cli_session_stop_co);

            uint32_t stop_step = 1;
            while (stop_step) {
              try {
                switch (stop_step) {
                  case 1:
                    timer_.cancel();
                    ++stop_step;
                  case 2:
                    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    ++stop_step;
                  case 3:
                    stream_.socket().cancel();
                    ++stop_step;
                  case 4:
                    stream_.socket().close();
                    ++stop_step;
                  case 5:
                    stream_.socket().release();
                    ++stop_step;
                  case 6:
                    stream_.cancel();
                    ++stop_step;
                  case 7:
                    stream_.close();
                    ++stop_step;
                  case 8:
                    stream_.release_socket();
                    ++stop_step;
                  default:
                    stop_step = 0;
                    break;
                }
              } catch (const std::exception& e) {
                DBG_PRINT("http cli session stop get exception at step %u, exception info: %s", stop_step, e.what());
                ++stop_step;
              }
            }
          });
    }

    bool CheckIdleAndUse() {
      bool is_idle = std::atomic_exchange(&idle_flag_, false);
      if (!is_idle || !run_flag_) return false;
      return true;
    }

    const std::atomic_bool& IsRunning() { return run_flag_; }

    boost::asio::strand<boost::asio::io_context::executor_type>& Strand() {
      return session_strand_;
    }

   private:
    std::shared_ptr<const AsioHttpClientProxy::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    std::atomic_bool idle_flag_ = false;
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::beast::tcp_stream stream_;
    boost::asio::steady_timer timer_;

    boost::beast::flat_buffer buffer_;
    bool tick_has_data_ = false;
    bool first_time_entry_ = true;
  };

  const AsioHttpClientProxy::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const AsioHttpClientProxy::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::list<std::shared_ptr<AsioHttpClientProxy::Session> > session_ptr_list_;
};

class AsioHttpClient : public std::enable_shared_from_this<AsioHttpClient> {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    size_t max_proxy_num = 1000;  // 最大proxy数

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.max_proxy_num < 10) cfg.max_proxy_num = 10;

      return cfg;
    }
  };

  /**
   * @brief http client构造函数
   *
   * @param io_ptr io_context
   * @param cfg 配置
   */
  AsioHttpClient(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioHttpClient::Cfg& cfg)
      : cfg_(AsioHttpClient::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {}

  ~AsioHttpClient() {}

  AsioHttpClient(const AsioHttpClient&) = delete;             ///< no copy
  AsioHttpClient& operator=(const AsioHttpClient&) = delete;  ///< no copy

  /**
   * @brief 获取http proxy
   * @note 非线程安全，需要在Strand下运行以保证线程安全
   * @param cfg http proxy的配置
   * @return std::shared_ptr<AsioHttpClientProxy> http proxy
   */
  std::shared_ptr<AsioHttpClientProxy> GetProxy(const AsioHttpClientProxy::Cfg& cfg) {
    if (!run_flag_) [[unlikely]]
      throw std::runtime_error("Http client is closed.");

    const size_t proxy_hash = std::hash<std::string>{}(cfg.host + cfg.service);

    auto itr = proxy_map_.find(proxy_hash);
    if (itr != proxy_map_.end()) {
      if (itr->second->IsRunning()) return itr->second;
      proxy_map_.erase(itr);
    }

    if (proxy_map_.size() >= cfg_.max_proxy_num)
      throw std::runtime_error("Http client proxy num reach the upper limit.");

    std::shared_ptr<AsioHttpClientProxy> proxy_ptr = std::make_shared<AsioHttpClientProxy>(io_ptr_, cfg);
    proxy_map_.emplace(proxy_hash, proxy_ptr);
    return proxy_ptr;
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
          ASIO_DEBUG_HANDLE(http_cli_stop_co);

          for (auto& itr : proxy_map_)
            itr.second->Stop();

          proxy_map_.clear();
        });
  }

  /**
   * @brief 是否在运行
   *
   * @return const std::atomic_bool&
   */
  const std::atomic_bool& IsRunning() { return run_flag_; }

  /**
   * @brief 调用GetProxy时的strand
   *
   * @return boost::asio::strand<boost::asio::io_context::executor_type>&
   */
  boost::asio::strand<boost::asio::io_context::executor_type>& Strand() {
    return mgr_strand_;
  }

 private:
  const AsioHttpClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;
  std::map<size_t, std::shared_ptr<AsioHttpClientProxy> > proxy_map_;
};

}  // namespace ytlib
