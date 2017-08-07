#pragma once
#include <ytlib/NetTools/TcpNetUtil.h>
#include <ytlib/Common/Util.h>
#include <boost/serialization/singleton.hpp>
#include <ytlib/LogService/LoggerServer.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/bounded_fifo_queue.hpp>
#include <boost/log/sinks/drop_on_overflow.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace ytlib {
	//此日志工具用于生产环境，调试环境的日志输出见<ytlib/Common/Util.h>中的YT_DEBUG_PRINTF

	//同步后端，不需要加锁
	class NetBackend : public boost::log::sinks::basic_formatted_sink_backend<
		char,boost::log::sinks::synchronized_feeding> {
	public:
		//目前只支持int型id。如果要改成string型id也很简单
		explicit NetBackend(uint32_t myid_,const TcpEp& logserver_ep_ ):
			sock(service), LogServerEp(logserver_ep_), ConnectFlag(false){
			header[0] = LogConnection::TCPHEAD1;
			header[1] = LogConnection::TCPHEAD2;
			header[2] = LogConnection::LOGHEAD1;
			header[3] = LogConnection::LOGHEAD2;
			logBuff.push_back(std::move(boost::asio::const_buffer(header, HEAD_SIZE)));
			//设置本机id。如果以后要添加本地信息也在此处添加拓展
			HostInfoSize = 4;
			HostInfoBuff = boost::shared_array<char>(new char[HostInfoSize]);
			set_buf_from_num(HostInfoBuff.get(), myid_);
			logBuff.push_back(std::move(boost::asio::const_buffer(HostInfoBuff.get(), HostInfoSize)));
			connect();
		}
		void consume(boost::log::record_view const& rec, std::string const& command_line) {
			//如果有连接才发送，否则不发送
			if (connect()) {
				set_buf_from_num(&header[4], static_cast<uint32_t>(command_line.size() + HostInfoSize));
				logBuff.push_back(std::move(boost::asio::buffer(command_line)));
				boost::system::error_code err;
				//发送失败则将ConnectFlag置为false
				sock.write_some(logBuff, err);
				logBuff.pop_back();
				if (err) {
					ConnectFlag = false;
					YT_DEBUG_PRINTF("send to log server failed : %s\n", err.message().c_str());
					return;
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
				YT_DEBUG_PRINTF("connect to log server failed : %s\n", err.message().c_str());
				return false;
			}
			ConnectFlag = true;
			return true;
		}

		boost::asio::io_service service;//全同步操作，所以不需要run
		TcpSocket sock;
		TcpEp LogServerEp;
		std::atomic_bool ConnectFlag;
		std::vector<boost::asio::const_buffer> logBuff;
		boost::shared_array<char> HostInfoBuff;
		uint32_t HostInfoSize;
		static const uint8_t HEAD_SIZE = 8;
		char header[HEAD_SIZE];//报头缓存

	};

	//日志控制中心。提供全局单例
	class LogControlCenter {
		typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> ConSink_t;
		typedef boost::log::sinks::synchronous_sink<NetBackend> NetSink_t;
	public:

		void EnableConsoleLog() {
			DisableConsoleLog();
			boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend = boost::make_shared<boost::log::sinks::text_ostream_backend>();
			backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
			ConSink = boost::shared_ptr<ConSink_t>(new ConSink_t(backend));
			//设置控制台日志格式
			ConSink->set_formatter(
				boost::log::expressions::stream
				<< "[" << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
				<< "]<" << boost::log::trivial::severity 
				<< ">:" << boost::log::expressions::smessage
			);
			boost::shared_ptr<boost::log::core> core = boost::log::core::get();
			core->add_sink(ConSink);
			boost::log::add_common_attributes();
		}
		void DisableConsoleLog() {
			if (ConSink) {
				boost::shared_ptr< boost::log::core > core = boost::log::core::get();
				core->remove_sink(ConSink);
			}
		}
		void EnableNetLog(uint32_t myid_, const TcpEp& logserver_ep_) {
			DisableConsoleLog();
			NetSink = boost::shared_ptr<NetSink_t >(new NetSink_t(myid_, logserver_ep_));
			//设置网络日志格式。因为会用于生产环境，所以无法给出scope属性
			NetSink->set_formatter(
				boost::log::expressions::stream 
				<< boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
				<< "\t" << boost::log::trivial::severity
				<< "\t" << boost::log::expressions::smessage
			);
			boost::shared_ptr< boost::log::core > core = boost::log::core::get();
			core->add_sink(NetSink);
			boost::log::add_common_attributes();
		}
		void DisableNetLog() {
			if (NetSink) {
				boost::shared_ptr< boost::log::core > core = boost::log::core::get();
				core->remove_sink(NetSink);
			}
		}

	private:
		boost::shared_ptr<NetSink_t> NetSink;
		boost::shared_ptr<ConSink_t> ConSink;
	};
	typedef boost::serialization::singleton<LogControlCenter> SingletonLogControlCenter;


	//一些宏定义
#define YT_LOG_TRACE	BOOST_LOG_TRIVIAL(trace)
#define YT_LOG_DEBUG	BOOST_LOG_TRIVIAL(debug)
#define YT_LOG_INFO		BOOST_LOG_TRIVIAL(info)
#define YT_LOG_WARNING	BOOST_LOG_TRIVIAL(warning)
#define YT_LOG_ERROR	BOOST_LOG_TRIVIAL(error)
#define YT_LOG_FATAL	BOOST_LOG_TRIVIAL(fatal)


#define YT_SET_LOG_LEVEL(lvl)  boost::log::core::get()->set_filter(boost::log::trivial::severity >=  boost::log::trivial::lvl);

	static void InitNetLog(uint32_t myid_, const TcpEp& logserver_ep_) {
		SingletonLogControlCenter::get_mutable_instance().EnableNetLog(myid_, logserver_ep_);
		SingletonLogControlCenter::get_mutable_instance().EnableConsoleLog();
		YT_SET_LOG_LEVEL(info);//默认info级别
	}
	static void StopNetLog() {
		SingletonLogControlCenter::get_mutable_instance().DisableNetLog();
		SingletonLogControlCenter::get_mutable_instance().DisableConsoleLog();
	}


}