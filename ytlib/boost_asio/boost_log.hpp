/**
 * @file boost_log.hpp
 * @brief 基于boost.log的日志
 * @note 基于boost.log的日志，封装了本地日志和远程日志
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <atomic>
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

#include "net_log.hpp"
#include "net_util.hpp"

namespace ytlib {

/**
 * @brief 基于boost.log的网络日志后端
 */
class NetLogBackend : public boost::log::sinks::basic_sink_backend<boost::log::sinks::synchronized_feeding>,
                      public boost::enable_shared_from_this<NetLogBackend> {
 public:
  /**
   * @brief 构造函数
   * @param[in] net_log_cli_ptr 网络日志客户端的智能指针
   */
  explicit NetLogBackend(std::shared_ptr<NetLogClient> net_log_cli_ptr) : net_log_cli_ptr_(net_log_cli_ptr) {
    if (!net_log_cli_ptr_)
      throw std::invalid_argument("net_log_cli_ptr can not be nullptr.");
  }

  ///析构函数
  ~NetLogBackend() {
    net_log_cli_ptr_->Stop();
  }

  /**
   * @brief 日志处理接口
   * @param[in] rec 单条日志内容
   */
  void consume(const boost::log::record_view& rec) {
    auto log_buf = std::make_shared<boost::asio::streambuf>();
    std::ostream oss(log_buf.get());

    oss << "[" << rec[boost::log::aux::default_attribute_names::timestamp()].extract<boost::posix_time::ptime>()
        << "][" << rec[boost::log::trivial::severity]
        << "][" << rec[boost::log::aux::default_attribute_names::thread_id()].extract<boost::log::attributes::current_thread_id::value_type>()
        << "]" << rec[boost::log::expressions::smessage];

    net_log_cli_ptr_->LogToSvr(log_buf);
  }

 private:
  std::shared_ptr<NetLogClient> net_log_cli_ptr_;
};

/**
 * @brief 日志控制中心
 * @note 提供全局单例。YTBL =  ytlib boost log
 */
class YTBLCtr {
 public:
  /**
   * @brief 获取全局单例
   * @return YTBLCtr全局单例
   */
  static YTBLCtr& Ins() {
    static YTBLCtr instance;
    return instance;
  }

  /**
   * @brief 开启控制台日志
   */
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

  /**
   * @brief 开启文件日志
   */
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

  /**
   * @brief 开启网络日志
   */
  void EnableNetLog(std::shared_ptr<NetLogClient> net_log_cli_ptr) {
    if (!std::atomic_exchange(&net_log_flag, true)) {
      auto sink = boost::make_shared<boost::log::sinks::synchronous_sink<NetLogBackend> >(boost::make_shared<NetLogBackend>(net_log_cli_ptr));

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

///设置日志级别：trace | debug | info | warning | error | fatal
#define YTBL_SET_LEVEL(lvl) (boost::log::core::get())->set_filter(boost::log::trivial::severity >= boost::log::trivial::lvl);

#define _YTBL_FMT_STRING(x) #x
#define YTBL_FMT_STRING(x) _YTBL_FMT_STRING(x)
#define YTBL_FMT "[" __FILE__ ":" YTBL_FMT_STRING(__LINE__) "@" << __FUNCTION__ << "]"

#define YTBL_TRACE BOOST_LOG_TRIVIAL(trace) << YTBL_FMT
#define YTBL_DEBUG BOOST_LOG_TRIVIAL(debug) << YTBL_FMT
#define YTBL_INFO BOOST_LOG_TRIVIAL(info) << YTBL_FMT
#define YTBL_WARN BOOST_LOG_TRIVIAL(warning) << YTBL_FMT
#define YTBL_ERROR BOOST_LOG_TRIVIAL(error) << YTBL_FMT
#define YTBL_FATAL BOOST_LOG_TRIVIAL(fatal) << YTBL_FMT

}  // namespace ytlib
