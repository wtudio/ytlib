#pragma once
#include <ytlib/NetTools/TcpNetUtil.h>
#include <ytlib/Common/Util.h>
#include <ytlib/LogService/LoggerServer.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>



namespace ytlib {
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

	//日志控制中心。提供全局单例。默认使用控制台日志。需要手动设置网络日志
	class LogControlCenter {
	public:
		LogControlCenter() {

		}
		~LogControlCenter() {

		}
		void EnableConsoleLog() {

		}
		void DisableNetLog() {

		}
		void EnableNetLog() {

		}
		void DisableConsoleLog() {

		}
		

	private:

	};

	//一些宏定义
#define YT_LOG_TRACE 
#define YT_LOG_DEBUG
#define YT_LOG_INFO
#define YT_LOG_WARNING
#define YT_LOG_ERROR
#define YT_LOG_FATAL

}