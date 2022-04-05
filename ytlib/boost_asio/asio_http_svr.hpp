#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include <boost/beast.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

namespace ytlib {

struct AsioHttpSvrCfg {
  uint16_t port = 80;  // 监听的端口

  std::string doc_root = ".";  // http页面根目录

  std::chrono::steady_clock::duration timer_dt = std::chrono::seconds(5);               // 定时器间隔
  std::chrono::steady_clock::duration max_no_data_duration = std::chrono::seconds(60);  // 最长无数据时间

  void Verify() {
    // 校验配置
    if (port > 65535) port = 80;

    if (timer_dt < std::chrono::seconds(1)) timer_dt = std::chrono::seconds(1);
    if (max_no_data_duration < timer_dt * 2) max_no_data_duration = timer_dt * 2;
  }
};

/**
 * @brief http服务器
 *
 */
class AsioHttpSvr : public std::enable_shared_from_this<AsioHttpSvr> {
 public:
  using HttpReq = boost::beast::http::request<boost::beast::http::string_body>;
  using HttpRsp = boost::beast::http::response<boost::beast::http::string_body>;
  using HttpHandleFunc = std::function<boost::asio::awaitable<HttpRsp>(const HttpReq&)>;

 public:
  AsioHttpSvr(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioHttpSvrCfg& cfg)
      : cfg_(cfg),
        io_ptr_(io_ptr),
        mgr_strand_(boost::asio::make_strand(*io_ptr_)) {
    cfg_.Verify();
  }

  ~AsioHttpSvr() {}

  AsioHttpSvr(const AsioHttpSvr&) = delete;             ///< no copy
  AsioHttpSvr& operator=(const AsioHttpSvr&) = delete;  ///< no copy

  /**
   * @brief 启动http服务器
   *
   */
  void Start() {
    acceptor_ptr_ = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_ptr_, boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4(), cfg_.port});

    auto self = shared_from_this();
    boost::asio::co_spawn(
        mgr_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          ASIO_DEBUG_HANDLE(http_svr_acceptor_co);
          try {
            while (run_flag_) {
              auto session_ptr = std::make_shared<HttpSession>(
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
   * @brief 停止http服务器
   * @note 需要在析构之前手动调用Stop
   */
  void Stop() {
    if (!std::atomic_exchange(&run_flag_, false)) return;

    auto self = shared_from_this();
    boost::asio::dispatch(
        mgr_strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(http_svr_stop_co);

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

  /**
   * @brief 注册处理函数
   * @note 目前只支持简单的路径匹配，暂不支持通配符等
   * @param pattern url路径
   * @param handle 处理函数
   */
  void RegisterHttpHandleFunc(std::string_view pattern, HttpHandleFunc&& handle) {
    if (handle) http_handle_map_.emplace(pattern, std::move(handle));
  }

 private:
  class HttpSession : public std::enable_shared_from_this<HttpSession> {
   public:
    HttpSession(boost::asio::ip::tcp::socket&& sock, std::shared_ptr<AsioHttpSvr> svr_ptr)
        : svr_ptr_(svr_ptr),
          strand_(boost::asio::make_strand(*(svr_ptr_->io_ptr_))),
          stream_(std::move(sock)),
          timer_(strand_) {
      const boost::asio::ip::tcp::endpoint& ep = stream_.socket().remote_endpoint();
      session_name_ = (ep.address().to_string() + "_" + std::to_string(ep.port()));
    }

    ~HttpSession() {}

    HttpSession(const HttpSession&) = delete;             ///< no copy
    HttpSession& operator=(const HttpSession&) = delete;  ///< no copy

    void Start() {
      DBG_PRINT("get a new connection from %s", session_name_.c_str());

      auto self = shared_from_this();
      // 请求处理协程
      boost::asio::co_spawn(
          strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            namespace http = boost::beast::http;

            ASIO_DEBUG_HANDLE(http_svr_session_recv_co);
            try {
              boost::beast::flat_buffer buffer;
              while (run_flag_) {
                http::request<http::string_body> req;
                co_await http::async_read(stream_, buffer, req, boost::asio::use_awaitable);

                if (req.method() != http::verb::get && req.method() != http::verb::head && req.method() != http::verb::post) {
                  auto rsp = BadRequestHandle(req, "UnSupport HTTP-method");
                  run_flag_ = !rsp.need_eof();
                  co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  continue;
                }

                if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos) {
                  auto rsp = BadRequestHandle(req, "Illegal request-target");
                  run_flag_ = !rsp.need_eof();
                  co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  continue;
                }

                // 处理handle类请求
                auto handle = GetHttpHandleFunc(std::string_view(req.target().data(), req.target().length()), svr_ptr_->http_handle_map_);
                if (handle) {
                  HttpRsp rsp;
                  try {
                    rsp = co_await handle(req);
                  } catch (const std::exception& e) {
                    rsp = ServerErrorHandle(req, e.what());
                  }
                  run_flag_ = !rsp.need_eof();
                  co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  continue;
                }

                std::string path = PathCat(svr_ptr_->cfg_.doc_root, req.target());
                if (req.target().back() == '/') path.append("index.html");

                boost::beast::error_code ec;
                http::file_body::value_type body;
                body.open(path.c_str(), boost::beast::file_mode::scan, ec);

                if (ec == boost::beast::errc::no_such_file_or_directory) {
                  auto rsp = NotFoundHandle(req, std::string_view(req.target().data(), req.target().length()));
                  run_flag_ = !rsp.need_eof();
                  co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  continue;
                }

                if (ec) {
                  auto rsp = ServerErrorHandle(req, ec.message());
                  run_flag_ = !rsp.need_eof();
                  co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
                  continue;
                }

                auto const size = body.size();

                // 处理head类请求
                if (req.method() == http::verb::head) {
                  http::response<http::empty_body> rsp{http::status::ok, req.version()};
                  rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                  rsp.set(http::field::content_type, MimeType(path));
                  rsp.content_length(size);
                  rsp.keep_alive(req.keep_alive());

                  run_flag_ = !rsp.need_eof();
                  co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
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

                run_flag_ = !rsp.need_eof();
                co_await http::async_write(stream_, rsp, boost::asio::use_awaitable);
              }
            } catch (std::exception& e) {
              DBG_PRINT("http session get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);

      // 定时器协程
      boost::asio::co_spawn(
          strand_,
          [this, self]() -> boost::asio::awaitable<void> {
            namespace chrono = std::chrono;

            ASIO_DEBUG_HANDLE(http_svr_session_timer_co);
            try {
              chrono::steady_clock::duration no_data_time_count = chrono::steady_clock::duration::zero();  // 当前无数据时间

              while (run_flag_) {
                timer_.expires_after(svr_ptr_->cfg_.timer_dt);
                co_await timer_.async_wait(boost::asio::use_awaitable);

                if (tick_has_data_) {
                  tick_has_data_ = false;
                  no_data_time_count = chrono::steady_clock::duration::zero();
                } else {
                  no_data_time_count += svr_ptr_->cfg_.timer_dt;
                  if (no_data_time_count >= svr_ptr_->cfg_.max_no_data_duration) {
                    DBG_PRINT("http session exit due to timeout(%llums), addr %s.", chrono::duration_cast<chrono::milliseconds>(no_data_time_count).count(), session_name_.c_str());
                    break;
                  }
                }
              }
            } catch (const std::exception& e) {
              DBG_PRINT("http session timer get exception and exit, addr %s, exception %s", session_name_.c_str(), e.what());
            }

            Stop();

            co_return;
          },
          boost::asio::detached);
    }

    void Stop() {
      if (!std::atomic_exchange(&run_flag_, false)) return;

      DBG_PRINT("http session stop, addr %s", session_name_.c_str());

      auto self = shared_from_this();
      boost::asio::dispatch(
          strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(http_svr_session_stop_co);

            if (stream_.socket().is_open()) {
              stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
              stream_.socket().cancel();
              stream_.socket().close();
              stream_.socket().release();
              stream_.cancel();
              stream_.close();
              stream_.release_socket();
            }

            timer_.cancel();
          });

      boost::asio::dispatch(
          svr_ptr_->mgr_strand_,
          [this, self]() {
            ASIO_DEBUG_HANDLE(http_svr_session_clear_co);

            auto finditr = svr_ptr_->session_ptr_set_.find(self);
            if (finditr != svr_ptr_->session_ptr_set_.end())
              svr_ptr_->session_ptr_set_.erase(finditr);
          });
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

    static HttpRsp BadRequestHandle(const HttpReq& req, std::string_view info) {
      namespace http = boost::beast::http;

      http::response<http::string_body> rsp{http::status::bad_request, req.version()};
      rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      rsp.set(http::field::content_type, "text/html");
      rsp.keep_alive(req.keep_alive());
      rsp.body() = info;
      rsp.prepare_payload();
      return rsp;
    }

    static HttpRsp NotFoundHandle(const HttpReq& req, std::string_view info) {
      namespace http = boost::beast::http;

      http::response<http::string_body> rsp{http::status::not_found, req.version()};
      rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      rsp.set(http::field::content_type, "text/html");
      rsp.keep_alive(req.keep_alive());
      rsp.body() = "The resource '" + std::string(info) + "' was not found.";
      rsp.prepare_payload();
      return rsp;
    }

    static HttpRsp ServerErrorHandle(const HttpReq& req, std::string_view info) {
      namespace http = boost::beast::http;

      http::response<http::string_body> rsp{http::status::internal_server_error, req.version()};
      rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      rsp.set(http::field::content_type, "text/html");
      rsp.keep_alive(req.keep_alive());
      rsp.body() = "An error occurred: '" + std::string(info) + "'";
      rsp.prepare_payload();
      return rsp;
    }

    static HttpHandleFunc GetHttpHandleFunc(std::string_view target, const std::map<std::string, HttpHandleFunc>& http_handle_map) {
      // todo 优化
      for (const auto& itr : http_handle_map) {
        if (target.size() < itr.first.size()) continue;
        if (target.substr(0, itr.first.size()) != itr.first) continue;
        return itr.second;
      }
      return HttpHandleFunc();
    }

   private:
    std::atomic_bool run_flag_ = true;
    std::shared_ptr<AsioHttpSvr> svr_ptr_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::beast::tcp_stream stream_;
    boost::asio::steady_timer timer_;

    std::string session_name_;

    bool tick_has_data_ = false;
  };

 private:
  std::atomic_bool run_flag_ = true;
  AsioHttpSvrCfg cfg_;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> mgr_strand_;  // session池操作strand
  std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr_;            // 监听器
  std::set<std::shared_ptr<HttpSession> > session_ptr_set_;                 // session池
  std::map<std::string, HttpHandleFunc> http_handle_map_;
};

}  // namespace ytlib
