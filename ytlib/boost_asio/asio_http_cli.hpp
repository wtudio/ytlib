#pragma once

#include <chrono>
#include <memory>

#include <boost/beast.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

namespace ytlib {

struct AsioHttpCliCfg {
  std::string host;            // 服务器域名或ip
  std::string service;         // 服务（如http、ftp）或端口号
  uint32_t max_retry_num = 5;  // 最大重试次数
};

/**
 * @brief http客户端
 *
 */
class AsioHttpCli : public std::enable_shared_from_this<AsioHttpCli> {
 public:
  using HttpReq = boost::beast::http::request<boost::beast::http::string_body>;
  using HttpRsp = boost::beast::http::response<boost::beast::http::string_body>;

 public:
  AsioHttpCli(std::shared_ptr<boost::asio::io_context> io_ptr, const AsioHttpCliCfg& cfg)
      : cfg_(cfg),
        io_ptr_(io_ptr),
        strand_(boost::asio::make_strand(*io_ptr_)),
        stream_(strand_) {}

  ~AsioHttpCli() {}

  AsioHttpCli(const AsioHttpCli&) = delete;             ///< no copy
  AsioHttpCli& operator=(const AsioHttpCli&) = delete;  ///< no copy

  /**
   * @brief http发收协程
   *
   * @param req 请求
   * @param timeout 超时，默认5s
   * @return boost::asio::awaitable<HttpRsp> 协程句柄
   */
  boost::asio::awaitable<HttpRsp> HttpSendRecv(const HttpReq& req,
                                               std::chrono::steady_clock::duration timeout = std::chrono::milliseconds(5000)) {
    namespace chrono = std::chrono;
    namespace asio = boost::asio;
    namespace http = boost::beast::http;

    chrono::steady_clock::time_point start_time_point = chrono::steady_clock::now();
    chrono::steady_clock::duration cur_duration;

    uint32_t retry_num = cfg_.max_retry_num;

    do {
      try {
        if (!stream_.socket().is_open()) {
          asio::ip::tcp::resolver resolver(strand_);
          auto const results = co_await resolver.async_resolve(cfg_.host, cfg_.service, asio::use_awaitable);

          cur_duration = chrono::steady_clock::now() - start_time_point;
          if (cur_duration >= timeout) [[unlikely]]
            throw std::logic_error("Timeout.");

          DBG_PRINT("async_connect, timeout %llums", chrono::duration_cast<chrono::milliseconds>(timeout - cur_duration).count());
          stream_.expires_after(timeout - cur_duration);
          co_await stream_.async_connect(results, asio::use_awaitable);
        }

        cur_duration = chrono::steady_clock::now() - start_time_point;
        if (cur_duration >= timeout) [[unlikely]]
          throw std::logic_error("Timeout.");

        DBG_PRINT("async_write, timeout %llums", chrono::duration_cast<chrono::milliseconds>(timeout - cur_duration).count());
        stream_.expires_after(timeout - cur_duration);
        co_await http::async_write(stream_, req, asio::use_awaitable);

        cur_duration = chrono::steady_clock::now() - start_time_point;
        if (cur_duration >= timeout) [[unlikely]]
          throw std::logic_error("Timeout.");

        DBG_PRINT("async_read, timeout %llums", chrono::duration_cast<chrono::milliseconds>(timeout - cur_duration).count());
        stream_.expires_after(timeout - cur_duration);
        HttpRsp rsp;
        co_await http::async_read(stream_, buffer_, rsp, asio::use_awaitable);

        co_return rsp;

      } catch (const std::exception& e) {
        DBG_PRINT("http send/recv get exception, addr %s, exception %s", TcpEp2Str(stream_.socket().remote_endpoint()).c_str(), e.what());
        CloseSocket();
      }
    } while (--retry_num);

    DBG_PRINT("http send/recv failed after %u retry.", cfg_.max_retry_num);
    throw std::logic_error("Http send/recv failed.");
  }

  /**
   * @brief 关闭客户端连接
   *
   */
  void Close() {
    auto self = shared_from_this();
    boost::asio::dispatch(
        strand_,
        [this, self]() {
          ASIO_DEBUG_HANDLE(http_cli_stop_co);
          CloseSocket();
        });
  }

  /**
   * @brief 获取客户端使用的strand
   *
   * @return boost::asio::strand<boost::asio::io_context::executor_type>&
   */
  boost::asio::strand<boost::asio::io_context::executor_type>& Strand() {
    return strand_;
  }

 private:
  void CloseSocket() {
    if (stream_.socket().is_open()) {
      stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
      stream_.socket().cancel();
      stream_.socket().close();
      stream_.socket().release();
      stream_.cancel();
      stream_.close();
      stream_.release_socket();
    }
  }

 private:
  AsioHttpCliCfg cfg_;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::beast::tcp_stream stream_;
  boost::beast::flat_buffer buffer_;
};

}  // namespace ytlib
