/**
 * @file Logger.h
 * @brief 基于boost.log的日志
 * @details 基于boost.log的日志，封装了本地日志和远程日志
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/Common/Util.h>
#include <ytlib/LogService/LoggerServer.h>
#include <ytlib/NetTools/TcpNetUtil.h>

#include <boost/core/null_deleter.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/bounded_fifo_queue.hpp>
#include <boost/log/sinks/drop_on_overflow.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace ytlib {

/**
 * @brief 基于boost.log的网络日志后端
 * 此日志工具用于生产环境，调试环境的日志输出见<ytlib/Common/Util.h>中的YT_DEBUG_PRINTF。
 * 在刚建立连接的第一包中发送id等host信息，并发送一个8字节的时间（boost::posix_time::ptime的内容）。
 * 然后后续每一包只发送1byte level信息+8byte 时间+n byte日志信息（不用增量是防止中间丢包）
 * 同步后端，不需要加锁
 */
class NetBackend : public boost::log::sinks::basic_sink_backend<
                       boost::log::sinks::synchronized_feeding> {
 public:
  //目前只支持int型id。如果要改成string型id也很简单
  explicit NetBackend(uint32_t myid_, const TcpEp& logserver_ep_) : sock(service), LogServerEp(logserver_ep_), ConnectFlag(false), m_bFirstLogFlag(true) {
    header[0] = LogConnection::TCPHEAD1;
    header[1] = LogConnection::TCPHEAD2;
    header[2] = LogConnection::LOGHEAD1;
    header[3] = LogConnection::LOGHEAD2;
    logBuff.push_back(boost::asio::const_buffer(header, HEAD_SIZE));
    //设置本机id和日志等级。如果以后要添加其他信息也在此处添加拓展
    HostInfoSize = 1 + 8 + 4;
    HostInfoBuff = boost::shared_array<char>(new char[HostInfoSize]);
    set_buf_from_num(&HostInfoBuff[9], myid_);
    logBuff.push_back(boost::asio::const_buffer(HostInfoBuff.get(), HostInfoSize));
    connect();
  }
  virtual ~NetBackend() {
    sock.close();
    logBuff.clear();
  }
  void consume(boost::log::record_view const& rec) {
    //如果有连接才发送，否则不发送
    if (connect()) {
      set_buf_from_num(&header[4], static_cast<uint32_t>(rec[boost::log::expressions::smessage]->size() + HostInfoSize));
      HostInfoBuff[0] = static_cast<uint8_t>(*rec[boost::log::trivial::severity]);
      const boost::posix_time::ptime* tnow = (rec[boost::log::aux::default_attribute_names::timestamp()].extract<boost::posix_time::ptime>()).get_ptr();
      transEndian(&HostInfoBuff[1], (char*)tnow, 8);
      logBuff.push_back(boost::asio::buffer(*rec[boost::log::expressions::smessage]));
      boost::system::error_code err;
      //发送失败则将ConnectFlag置为false
      sock.write_some(logBuff, err);
      logBuff.pop_back();  //弹出msg
      if (err) {
        ConnectFlag = false;
        m_bFirstLogFlag = true;
        YT_DEBUG_PRINTF("send to log server failed : %s", err.message().c_str());
        return;
      }
      if (m_bFirstLogFlag) {
        logBuff.pop_back();  //弹出第一包内容
        HostInfoSize = 1 + 8;
        HostInfoBuff = boost::shared_array<char>(new char[HostInfoSize]);
        logBuff.push_back(boost::asio::const_buffer(HostInfoBuff.get(), HostInfoSize));
        m_bFirstLogFlag = false;
      }
    }
  }

 private:
  bool connect() {
    if (ConnectFlag) return true;
    sock.open(boost::asio::ip::tcp::v4());
    boost::system::error_code err;
    sock.connect(LogServerEp, err);
    if (err) {
      YT_DEBUG_PRINTF("connect to log server failed : %s", err.message().c_str());
      return false;
    }
    ConnectFlag = true;
    return true;
  }

  boost::asio::io_service service;  //全同步操作，所以不需要run
  TcpSocket sock;
  TcpEp LogServerEp;
  std::atomic_bool ConnectFlag;
  std::vector<boost::asio::const_buffer> logBuff;
  boost::shared_array<char> HostInfoBuff;
  uint32_t HostInfoSize;
  static const uint8_t HEAD_SIZE = 8;
  char header[HEAD_SIZE];  //报头缓存
  bool m_bFirstLogFlag;
};

/**
 * @brief 日志控制中心
 * 提供全局单例
 */
class LogControlCenter {
  typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> ConSink_t;
  typedef boost::log::sinks::synchronous_sink<NetBackend> NetSink_t;

 public:
  LogControlCenter() {
    //要添加什么属性在这里添加
    //boost::log::add_common_attributes();
    (boost::log::core::get())->add_global_attribute(boost::log::aux::default_attribute_names::timestamp(), boost::log::attributes::local_clock());
  }

  void EnableConsoleLog() {
    DisableConsoleLog();
    boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend = boost::make_shared<boost::log::sinks::text_ostream_backend>();
    backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
    ConSink = boost::make_shared<ConSink_t>(backend);
    //设置控制台日志格式
    ConSink->set_formatter(
        boost::log::expressions::stream
        << "[" << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
        << "]<" << boost::log::trivial::severity
        << ">:" << boost::log::expressions::smessage);
    (boost::log::core::get())->add_sink(ConSink);
  }
  void DisableConsoleLog() {
    if (ConSink) {
      (boost::log::core::get())->remove_sink(ConSink);
      ConSink.reset();
    }
  }
  void EnableNetLog(uint32_t myid_, const TcpEp& logserver_ep_) {
    DisableConsoleLog();
    NetSink = boost::make_shared<NetSink_t>(myid_, logserver_ep_);
    (boost::log::core::get())->add_sink(NetSink);
  }
  void DisableNetLog() {
    if (NetSink) {
      (boost::log::core::get())->remove_sink(NetSink);
      NetSink.reset();
    }
  }

 private:
  boost::shared_ptr<NetSink_t> NetSink;
  boost::shared_ptr<ConSink_t> ConSink;
};
typedef boost::serialization::singleton<LogControlCenter> SingletonLogControlCenter;

//日志宏定义
#define YT_LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define YT_LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define YT_LOG_INFO BOOST_LOG_TRIVIAL(info)
#define YT_LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define YT_LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define YT_LOG_FATAL BOOST_LOG_TRIVIAL(fatal)

#define YT_SET_LOG_LEVEL(lvl) (boost::log::core::get())->set_filter(boost::log::trivial::severity >= boost::log::trivial::lvl);

///网络日志初始化。不要重复init
inline void InitNetLog(uint32_t myid_, const TcpEp& logserver_ep_) {
  SingletonLogControlCenter::get_mutable_instance().EnableNetLog(myid_, logserver_ep_);
  SingletonLogControlCenter::get_mutable_instance().EnableConsoleLog();
  YT_SET_LOG_LEVEL(info);  //默认info级别
}
///关闭网络日志
inline void StopNetLog() {
  SingletonLogControlCenter::get_mutable_instance().DisableNetLog();
  SingletonLogControlCenter::get_mutable_instance().DisableConsoleLog();
}

}  // namespace ytlib