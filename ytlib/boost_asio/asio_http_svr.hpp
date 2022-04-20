/**
 * @file asio_http_svr.hpp
 * @brief 基于boost.beast的http客户端
 * @note 基于boost.beast的http客户端
 * @author WT
 * @date 2022-04-18
 */
#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include <boost/beast.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/http_dispatcher.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

namespace ytlib {

/**
 * @brief http服务器
 *
 */
class AsioHttpServer : public std::enable_shared_from_this<AsioHttpServer> {
 public:
  using HttpReq = boost::beast::http::request<boost::beast::http::dynamic_body>;

 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    uint16_t port = 80;  // 监听的端口

    std::string doc_root = ".";  // http页面根目录

    size_t max_session_num = 1000000;                                            // 最大连接数
    std::chrono::steady_clock::duration mgr_timer_dt = std::chrono::seconds(5);  // 管理协程定时器间隔

    std::chrono::steady_clock::duration timer_dt = std::chrono::seconds(5);               // 定时器间隔
    std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(60);  // 最长无数据时间

    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.port > 65535) cfg.port = 80;

      if (cfg.max_session_num < 1) cfg.max_session_num = 1;
      if (cfg.max_session_num > boost::asio::ip::tcp::acceptor::max_listen_connections)
        cfg.max_session_num = boost::asio::ip::tcp::acceptor::max_listen_connections;
      if (cfg.mgr_timer_dt < std::chrono::seconds(1)) cfg.mgr_timer_dt = std::chrono::seconds(1);

      if (cfg.timer_dt < std::chrono::seconds(1)) cfg.timer_dt = std::chrono::seconds(1);
      if (cfg.max_no_data_duration < cfg.timer_dt * 2) cfg.max_no_data_duration = cfg.timer_dt * 2;

      return cfg;
    }
  };

  /**
   * @brief http server构造函数
   *
   * @param io_ptr io_context
   * @param cfg 配置
   */
  AsioHttpServer(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioHttpServer::Cfg& cfg)
      : cfg_(AsioHttpServer::Cfg::Verify(cfg)),
        io_ptr_(io_ptr),
        session_cfg_ptr_(std::make_shared<const AsioHttpServer::SessionCfg>(cfg_)),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)),
        acceptor_(mgr_strand_, boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4(), cfg_.port}),
        acceptor_timer_(mgr_strand_),
        mgr_timer_(mgr_strand_),
        http_dispatcher_ptr_(std::make_shared<HttpDispatcher<boost::asio::awaitable<void>(std::shared_ptr<AsioHttpServer::Session>, const HttpReq&)>>()) {}

  ~AsioHttpServer() {}

  AsioHttpServer(const AsioHttpServer&) = delete;             ///< no copy
  AsioHttpServer& operator=(const AsioHttpServer&) = delete;  ///< no copy

  /**
   * @brief 启动http服务器
   *
   */
  void Start() {
    auto self = this->shared_from_this();
    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(http_svr_acceptor_co);

          while (run_flag_) {
            try {
              // 如果链接数达到上限，则等待一段时间再试
              if (session_ptr_list_.size() >= cfg_.max_session_num) {
                acceptor_timer_.expires_after(cfg_.mgr_timer_dt);
                co_await acceptor_timer_.async_wait(boost::asio::use_awaitable);
                continue;
              }

              auto session_ptr = std::make_shared<AsioHttpServer::Session>(boost::asio::make_strand(*io_ptr_), session_cfg_ptr_, http_dispatcher_ptr_);
              co_await acceptor_.async_accept(session_ptr->Socket(), boost::asio::use_awaitable);
              session_ptr->Start();

              session_ptr_list_.emplace_back(session_ptr);

            } catch (const std::exception& e) {
              DBG_PRINT("http svr accept connection get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);

    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(http_svr_timer_co);
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
              DBG_PRINT("http svr timer get exception and exit, exception info: %s", e.what());
            }
          }

          Stop();

          co_return;
        },
        boost::asio::detached);
  }

  /**
   * @brief 停止http服务器
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = this->shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(http_svr_stop_co);

          try {
            acceptor_timer_.cancel();
          } catch (const std::exception& e) {
            DBG_PRINT("http svr acceptor timer cancel get exception, exception info: %s", e.what());
          }

          try {
            mgr_timer_.cancel();
          } catch (const std::exception& e) {
            DBG_PRINT("http svr mgr timer cancel get exception, exception info: %s", e.what());
          }

          try {
            acceptor_.cancel();
          } catch (const std::exception& e) {
            DBG_PRINT("http svr acceptor cancel get exception, exception info: %s", e.what());
          }

          try {
            acceptor_.close();
          } catch (const std::exception& e) {
            DBG_PRINT("http svr acceptor close get exception, exception info: %s", e.what());
          }

          try {
            acceptor_.release();
          } catch (const std::exception& e) {
            DBG_PRINT("http svr acceptor release get exception, exception info: %s", e.what());
          }

          for (auto& session_ptr : session_ptr_list_)
            session_ptr->Stop();

          session_ptr_list_.clear();
        });
  }

  /**
   * @brief 注册自定义http处理接口
   *
   * @tparam RspBodyType 返回包body类型
   * @param pattern http uri
   * @param handle http处理接口
   */
  template <typename RspBodyType = boost::beast::http::string_body>
  void RegisterHttpHandleFunc(std::string_view pattern,
                              std::function<boost::asio::awaitable<boost::beast::http::response<RspBodyType>>(const HttpReq&)>&& handle) {
    http_dispatcher_ptr_->RegisterHttpHandle(pattern, Session::GenHttpHandle(std::move(handle)));
  }

  /**
   * @brief 注册自定义http处理接口
   *
   * @tparam RspBodyType 返回包body类型
   * @param pattern http uri
   * @param handle http处理接口
   */
  template <typename RspBodyType = boost::beast::http::string_body>
  void RegisterHttpHandleFunc(std::string_view pattern,
                              const std::function<boost::asio::awaitable<boost::beast::http::response<RspBodyType>>(const HttpReq&)>& handle) {
    auto h = handle;
    http_dispatcher_ptr_->RegisterHttpHandle(pattern, Session::GenHttpHandle(std::move(h)));
  }

 private:
  struct SessionCfg {
    SessionCfg(const Cfg& cfg)
        : doc_root(cfg.doc_root),
          timer_dt(cfg.timer_dt),
          max_no_data_duration(cfg.max_no_data_duration) {}

    std::string doc_root;

    std::chrono::steady_clock::duration timer_dt;
    std::chrono::steady_clock::duration max_no_data_duration;
  };

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(boost::asio::strand<boost::asio::io_context::executor_type> session_strand,
            std::shared_ptr<const AsioHttpServer::SessionCfg> session_cfg_ptr,
            std::shared_ptr<HttpDispatcher<boost::asio::awaitable<void>(std::shared_ptr<AsioHttpServer::Session>, const HttpReq&)>> http_dispatcher_ptr)
        : session_cfg_ptr_(session_cfg_ptr),
          session_strand_(session_strand),
          stream_(session_strand_),
          timer_(session_strand_),
          http_dispatcher_ptr_(http_dispatcher_ptr) {}

    ~Session() {}

    Session(const Session&) = delete;             ///< no copy
    Session& operator=(const Session&) = delete;  ///< no copy

    void Start() {
      const boost::asio::ip::tcp::endpoint& ep = stream_.socket().remote_endpoint();
      session_name_ = (ep.address().to_string() + "_" + std::to_string(ep.port()));

      auto self = this->shared_from_this();

      // 请求处理协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(http_svr_session_recv_co);

            try {
              namespace http = boost::beast::http;
              boost::beast::flat_buffer buffer;

              while (run_flag_ && !close_connect_flag_) {
                HttpReq req;
                size_t read_data_size = co_await http::async_read(stream_, buffer, req, boost::asio::use_awaitable);
                DBG_PRINT("http svr session async read %llu bytes", read_data_size);

                // 检查bad req
                std::string_view bad_req_check_ret = CheckBadRequest(req);
                if (!bad_req_check_ret.empty()) {
                  const auto& rsp = BadRequestHandle(req, bad_req_check_ret);
                  close_connect_flag_ = rsp.need_eof();

                  DBG_PRINT("http svr session get bad request, err msg: %s, close_connect_flag: %d", bad_req_check_ret.data(), close_connect_flag_);
                  size_t write_data_size = co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  DBG_PRINT("http svr session async write %llu bytes", write_data_size);
                  continue;
                }

                // 处理handle类请求
                const auto& handle = http_dispatcher_ptr_->GetHttpHandle(std::string_view(req.target().data(), req.target().length()));
                if (handle) {
                  co_await handle(self, req);
                  continue;
                }

                std::string path = PathCat(session_cfg_ptr_->doc_root, req.target());
                if (req.target().back() == '/') path.append("index.html");

                boost::beast::error_code ec;
                http::file_body::value_type body;
                body.open(path.c_str(), boost::beast::file_mode::scan, ec);

                if (ec == boost::beast::errc::no_such_file_or_directory) {
                  const auto& rsp = NotFoundHandle(req, std::string_view(req.target().data(), req.target().length()));
                  close_connect_flag_ = rsp.need_eof();

                  DBG_PRINT("http svr session get 404, close_connect_flag: %d", close_connect_flag_);
                  size_t write_data_size = co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  DBG_PRINT("http svr session async write %llu bytes", write_data_size);
                  continue;
                }

                if (ec) {
                  const auto& rsp = ServerErrorHandle(req, ec.message());
                  close_connect_flag_ = rsp.need_eof();

                  DBG_PRINT("http svr session get server error, err msg: %s, close_connect_flag: %d", ec.message().c_str(), close_connect_flag_);
                  size_t write_data_size = co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  DBG_PRINT("http svr session async write %llu bytes", write_data_size);
                  continue;
                }

                const auto size = body.size();

                // 处理head类请求
                if (req.method() == http::verb::head) {
                  http::response<http::empty_body> rsp{http::status::ok, req.version()};
                  rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                  rsp.set(http::field::content_type, MimeType(path));
                  rsp.content_length(size);
                  rsp.keep_alive(req.keep_alive());

                  close_connect_flag_ = rsp.need_eof();

                  DBG_PRINT("http svr session get head request, close_connect_flag: %d", close_connect_flag_);
                  size_t write_data_size = co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  DBG_PRINT("http svr session async write %llu bytes", write_data_size);
                  continue;
                }

                // 处理文件类请求
                http::response<http::file_body> rsp{
                    std::piecewise_construct,
                    std::make_tuple(std::move(body)),
                    std::make_tuple(http::status::ok, req.version())};
                rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                rsp.set(http::field::content_type, MimeType(path));
                rsp.content_length(size);
                rsp.keep_alive(req.keep_alive());

                close_connect_flag_ = rsp.need_eof();

                DBG_PRINT("http svr session get file request, close_connect_flag: %d", close_connect_flag_);
                size_t write_data_size = co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                DBG_PRINT("http svr session async write %llu bytes", write_data_size);
              }
            } catch (std::exception& e) {
              DBG_PRINT("http svr session get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          session_strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            ASIO_DEBUG_HANDLE(http_svr_session_timer_co);

            try {
              namespace chrono = std::chrono;
              chrono::steady_clock::time_point last_data_time_point = chrono::steady_clock::now();  // 上次有数据时间

              while (run_flag_) {
                timer_.expires_after(session_cfg_ptr_->timer_dt);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                  last_data_time_point = chrono::steady_clock::now();

                } else {
                  chrono::steady_clock::duration no_data_duration = chrono::steady_clock::now() - last_data_time_point;
                  if (no_data_duration >= session_cfg_ptr_->max_no_data_duration) {
                    DBG_PRINT("http svr session exit due to timeout(%llums), addr %s.", chrono::duration_cast<chrono::milliseconds>(no_data_duration).count(), session_name_.c_str());
                    break;
                  }
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session timer get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);
    }

    void Stop() {
      if (!std::atomic_exchange(&run_flag_, false)) return;

      auto self = this->shared_from_this();
      boost::asio::dispatch(
          session_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(http_svr_session_stop_co);

            try {
              timer_.cancel();
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session timer cancel get exception, exception info: %s", e.what());
            }

            try {
              stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session socket shutdown get exception, exception info: %s", e.what());
            }

            try {
              stream_.socket().cancel();
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session socket cancel get exception, exception info: %s", e.what());
            }

            try {
              stream_.socket().close();
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session socket close get exception, exception info: %s", e.what());
            }

            try {
              stream_.socket().release();
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session socket release get exception, exception info: %s", e.what());
            }

            try {
              stream_.cancel();
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session stream cancel get exception, exception info: %s", e.what());
            }

            try {
              stream_.close();
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session stream close get exception, exception info: %s", e.what());
            }

            try {
              stream_.release_socket();
            } catch (const std::exception& e) {
              DBG_PRINT("http svr session stream release socket get exception, exception info: %s", e.what());
            }
          });
    }

    boost::asio::ip::tcp::socket& Socket() { return stream_.socket(); }

    const std::atomic_bool& IsRunning() { return run_flag_; }

    template <typename RspBodyType = boost::beast::http::string_body>
    static std::function<boost::asio::awaitable<void>(std::shared_ptr<Session>, const HttpReq&)> GenHttpHandle(
        std::function<boost::asio::awaitable<boost::beast::http::response<RspBodyType>>(const HttpReq&)>&& handle) {
      return [h = std::move(handle)](std::shared_ptr<Session> session_ptr, const HttpReq& req) -> boost::asio::awaitable<void> {
        boost::beast::http::response<RspBodyType> handle_rsp;
        std::string exp_info;
        try {
          handle_rsp = co_await h(req);
        } catch (const std::exception& e) {
          exp_info = e.what();
        }

        if (!exp_info.empty()) {
          const auto& rsp = ServerErrorHandle(req, exp_info);
          session_ptr->close_connect_flag_ = rsp.need_eof();

          DBG_PRINT("http svr session custom handle request get exp, close_connect_flag: %d", session_ptr->close_connect_flag_);
          size_t write_data_size = co_await boost::beast::http::async_write(session_ptr->stream_, rsp, boost::asio::use_awaitable);
          DBG_PRINT("http svr session async write %llu bytes", write_data_size);
        } else {
          session_ptr->close_connect_flag_ = handle_rsp.need_eof();

          DBG_PRINT("http svr session custom handle request, close_connect_flag: %d", session_ptr->close_connect_flag_);
          size_t write_data_size = co_await boost::beast::http::async_write(session_ptr->stream_, handle_rsp, boost::asio::use_awaitable);
          DBG_PRINT("http svr session async write %llu bytes", write_data_size);
        }
      };
    }

   private:
    static boost::beast::string_view MimeType(boost::beast::string_view path) {
      using boost::beast::iequals;
      auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == boost::beast::string_view::npos)
          return boost::beast::string_view{};
        return path.substr(pos);
      }();
      if (iequals(ext, ".htm")) return "text/html";
      if (iequals(ext, ".html")) return "text/html";
      if (iequals(ext, ".php")) return "text/html";
      if (iequals(ext, ".css")) return "text/css";
      if (iequals(ext, ".txt")) return "text/plain";
      if (iequals(ext, ".js")) return "application/javascript";
      if (iequals(ext, ".json")) return "application/json";
      if (iequals(ext, ".xml")) return "application/xml";
      if (iequals(ext, ".swf")) return "application/x-shockwave-flash";
      if (iequals(ext, ".flv")) return "video/x-flv";
      if (iequals(ext, ".png")) return "image/png";
      if (iequals(ext, ".jpe")) return "image/jpeg";
      if (iequals(ext, ".jpeg")) return "image/jpeg";
      if (iequals(ext, ".jpg")) return "image/jpeg";
      if (iequals(ext, ".gif")) return "image/gif";
      if (iequals(ext, ".bmp")) return "image/bmp";
      if (iequals(ext, ".ico")) return "image/vnd.microsoft.icon";
      if (iequals(ext, ".tiff")) return "image/tiff";
      if (iequals(ext, ".tif")) return "image/tiff";
      if (iequals(ext, ".svg")) return "image/svg+xml";
      if (iequals(ext, ".svgz")) return "image/svg+xml";
      return "application/text";
    }

    static std::string PathCat(boost::beast::string_view base, boost::beast::string_view path) {
      if (base.empty())
        return std::string(path);
      std::string result(base);
#ifdef _WIN32
      char constexpr path_separator = '\\';
      if (result.back() == path_separator)
        result.resize(result.size() - 1);
      result.append(path.data(), path.size());
      for (auto& c : result)
        if (c == '/')
          c = path_separator;
#else
      char constexpr path_separator = '/';
      if (result.back() == path_separator)
        result.resize(result.size() - 1);
      result.append(path.data(), path.size());
#endif
      return result;
    }

    static std::string_view CheckBadRequest(const HttpReq& req) {
      namespace http = boost::beast::http;

      // HTTP method 检查
      if (req.method() != http::verb::get &&
          req.method() != http::verb::head &&
          req.method() != http::verb::post) {
        const static std::string err_str = "UnSupport HTTP-method";
        return err_str;
      }

      // uri 检查
      if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos) {
        const static std::string err_str = "Illegal request-target";
        return err_str;
      }

      return std::string_view();
    }

    static boost::beast::http::response<boost::beast::http::string_body> BadRequestHandle(const HttpReq& req, std::string_view info) {
      namespace http = boost::beast::http;

      http::response<http::string_body> rsp{http::status::bad_request, req.version()};
      rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      rsp.set(http::field::content_type, "text/html");
      rsp.keep_alive(req.keep_alive());
      rsp.body() = info;
      rsp.prepare_payload();
      return rsp;
    }

    static boost::beast::http::response<boost::beast::http::string_body> NotFoundHandle(const HttpReq& req, std::string_view info) {
      namespace http = boost::beast::http;

      http::response<http::string_body> rsp{http::status::not_found, req.version()};
      rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      rsp.set(http::field::content_type, "text/html");
      rsp.keep_alive(req.keep_alive());
      rsp.body() = "The resource '" + std::string(info) + "' was not found.";
      rsp.prepare_payload();
      return rsp;
    }

    static boost::beast::http::response<boost::beast::http::string_body> ServerErrorHandle(const HttpReq& req, std::string_view info) {
      namespace http = boost::beast::http;

      http::response<http::string_body> rsp{http::status::internal_server_error, req.version()};
      rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      rsp.set(http::field::content_type, "text/html");
      rsp.keep_alive(req.keep_alive());
      rsp.body() = "An error occurred: '" + std::string(info) + "'";
      rsp.prepare_payload();
      return rsp;
    }

   private:
    std::shared_ptr<const AsioHttpServer::SessionCfg> session_cfg_ptr_;
    std::atomic_bool run_flag_ = true;
    boost::asio::strand<boost::asio::io_context::executor_type> session_strand_;
    boost::beast::tcp_stream stream_;
    boost::asio::steady_timer timer_;

    std::shared_ptr<HttpDispatcher<boost::asio::awaitable<void>(std::shared_ptr<AsioHttpServer::Session>, const HttpReq&)>> http_dispatcher_ptr_;
    std::string session_name_;
    bool close_connect_flag_ = false;
    bool tick_has_data_ = false;
  };

 private:
  const AsioHttpServer::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<boost::asio::io_context> io_ptr_;

  std::shared_ptr<const AsioHttpServer::SessionCfg> session_cfg_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;  // session池操作strand
  boost::asio::ip::tcp::acceptor acceptor_;                                 // 监听器
  boost::asio::steady_timer acceptor_timer_;                                // 连接满时监听器的sleep定时器
  boost::asio::steady_timer mgr_timer_;                                     // 管理session池的定时器
  std::list<std::shared_ptr<AsioHttpServer::Session>> session_ptr_list_;    // session池

  std::shared_ptr<HttpDispatcher<boost::asio::awaitable<void>(std::shared_ptr<AsioHttpServer::Session>, const HttpReq&)>> http_dispatcher_ptr_;
};

}  // namespace ytlib
