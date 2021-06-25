/**
 * @file boost_log.hpp
 * @brief 基于boost.log的日志
 * @details 基于boost.log的日志，封装了本地日志和远程日志
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <memory>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/bounded_fifo_queue.hpp>
#include <boost/log/sinks/drop_on_overflow.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/shared_ptr.hpp>

#include "net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

/**
 * @brief 基于boost.log的网络日志后端
 * 使用一个定时器协程不断的去检查并连接远端日志服务器。如果暂时还没连上远端日志服务器，则日志会被丢弃
 */
class NetLogBackend : public boost::log::sinks::basic_sink_backend<boost::log::sinks::synchronized_feeding>,
                      public boost::enable_shared_from_this<NetLogBackend> {
 public:
  explicit NetLogBackend(boost::asio::io_context* io_ptr,
                         const TcpEp& log_svr_ep) : strand_(boost::asio::make_strand(*io_ptr)),
                                                    log_svr_ep_(log_svr_ep),
                                                    sock_(strand_) {}

  ~NetLogBackend() {
    if (sock_.is_open())
      sock_.close();
  }

  // 同步日志处理函数
  void consume(const boost::log::record_view& rec) {
    auto buf_ptr = std::make_shared<boost::asio::streambuf>();
    std::ostream oss(buf_ptr.get());

    oss << "[" << rec[boost::log::aux::default_attribute_names::timestamp()].extract<boost::posix_time::ptime>()
        << "][" << rec[boost::log::trivial::severity]
        << "][" << rec[boost::log::aux::default_attribute_names::thread_id()].extract<boost::log::attributes::current_thread_id::value_type>()
        << "]" << rec[boost::log::expressions::smessage];

    auto self = shared_from_this();
    boost::asio::co_spawn(
        strand_,
        [this, self, buf_ptr]() -> boost::asio::awaitable<void> {
          try {
            if (!sock_.is_open()) {
              co_await sock_.async_connect(log_svr_ep_, boost::asio::use_awaitable);
            }

            while (buf_ptr->size()) {
              std::size_t n = co_await sock_.async_write_some(buf_ptr->data(), boost::asio::use_awaitable);
              buf_ptr->consume(n);
            }

          } catch (const std::exception& e) {
            std::cerr << "send log to svr get exception, addr:" << sock_.remote_endpoint() << ", exception:" << e.what() << '\n';
          }
          co_return;
        },
        boost::asio::detached);
  }

  void flush() {
    return;
  }

 private:
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  TcpEp log_svr_ep_;
  TcpSocket sock_;
};

/**
 * @brief 日志控制中心
 * 提供全局单例
 */
class YTBLCtr {
 public:
  // 单例
  static YTBLCtr& Ins() {
    static YTBLCtr instance;
    return instance;
  }

  void EnableConsoleLog() {
    if (!std::atomic_exchange(&con_log_flag, true)) {
      boost::log::add_console_log(
          std::clog,
          boost::log::keywords::format =
              (boost::log::expressions::stream
               << "[" << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
               << "][" << boost::log::trivial::severity
               << "][" << boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID")
               << "]" << boost::log::expressions::smessage));
    }
  }

  void EnableFileLog(const std::string& base_file_name_) {
    if (!std::atomic_exchange(&file_log_flag, true)) {
      boost::log::add_file_log(
          boost::log::keywords::file_name = base_file_name_ + "_%Y%m%d_%H%M%S.log",
          boost::log::keywords::rotation_size = 10 * 1024 * 1024,
          boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
          boost::log::keywords::auto_flush = true,
          boost::log::keywords::format =
              (boost::log::expressions::stream
               << "[" << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
               << "][" << boost::log::trivial::severity
               << "][" << boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID")
               << "]" << boost::log::expressions::smessage));
    }
  }

  void EnableNetLog(boost::asio::io_context& io, const TcpEp& log_svr_ep) {
    if (!std::atomic_exchange(&net_log_flag, true)) {
      auto sink = boost::make_shared<boost::log::sinks::synchronous_sink<NetLogBackend>>(&io, log_svr_ep);

      boost::log::core::get()->add_sink(sink);
    }
  }

 private:
  YTBLCtr() : con_log_flag(false), file_log_flag(false), net_log_flag(false) {
    //要添加什么属性在这里添加
    //boost::log::add_common_attributes();
    auto core_ptr = boost::log::core::get();
    core_ptr->add_global_attribute(
        boost::log::aux::default_attribute_names::timestamp(), boost::log::attributes::local_clock());
    core_ptr->add_global_attribute(
        boost::log::aux::default_attribute_names::thread_id(), boost::log::attributes::current_thread_id());

    core_ptr->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
  }

  std::atomic_bool con_log_flag;
  std::atomic_bool file_log_flag;
  std::atomic_bool net_log_flag;
};

//日志宏定义，YTBL=ytlib boost log
#define YTBL_FMT "[" __FILENAME__ ":" STRING(__LINE__) "@" << __FUNCTION__ << "]"
#define YTBL_TRACE BOOST_LOG_TRIVIAL(trace) << YTBL_FMT
#define YTBL_DEBUG BOOST_LOG_TRIVIAL(debug) << YTBL_FMT
#define YTBL_INFO BOOST_LOG_TRIVIAL(info) << YTBL_FMT
#define YTBL_WARN BOOST_LOG_TRIVIAL(warning) << YTBL_FMT
#define YTBL_ERROR BOOST_LOG_TRIVIAL(error) << YTBL_FMT
#define YTBL_FATAL BOOST_LOG_TRIVIAL(fatal) << YTBL_FMT

#define YTBL_SET_LEVEL(lvl) (boost::log::core::get())->set_filter(boost::log::trivial::severity >= boost::log::trivial::lvl);

}  // namespace ytlib
