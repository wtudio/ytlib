#pragma once
#include <ytlib/Common/Util.h>
#include <ytlib/Common/FileSystem.h>
#include <ytlib/NetTools/TcpConnectionPool.h>
#include <boost/shared_array.hpp>
#include <ytlib/SupportTools/ChannelBase.h>
#include <boost/date_time/posix_time/posix_time.hpp>  
#include <ytlib/NetTools/SharedBuf.h>
#include <boost/log/trivial.hpp>
#include <fstream>

namespace ytlib {
	/*
	目前版本日志服务器以目标机器id+ip:port为标签建立table
	日志内容只有time、level、msg
	初始化时给定一个路径，日志服务器每新连接一个客户机就在此目录下建立id_ip_port路径
	并以time_ip_id.txt的名称建立日志文件。time为日志文件建立的时间
	当满足一定条件（达到某个时间点、日志文件大小过大）时新建一个数据库
	*/

	//连接类。tag始终为 LG
	class LogConnection :public ConnectionBase {
	public:
		enum {
			LOGHEAD1 = 'L',
			LOGHEAD2 = 'G'
		};
		
		LogConnection(boost::asio::io_service& io_, std::function<void(const TcpEp &)> errcb_, tpath const *plogPath_) :
			ConnectionBase(io_, errcb_), plogPath(plogPath_), m_bFirstLogFlag(true), m_logQueue(1000), 
			m_handleThread(std::bind(&LogConnection::logHandel,this)){

		}
		virtual ~LogConnection() {
			stopflag = true;
			m_logQueue.Stop();
			m_handleThread.join();
		}

	private:
		void do_read_head() {
			boost::asio::async_read(sock, boost::asio::buffer(header, HEAD_SIZE), boost::asio::transfer_exactly(HEAD_SIZE),
				std::bind(&LogConnection::on_read_head, this, std::placeholders::_1, std::placeholders::_2));
		}
		//读取解析报头
		void on_read_head(const boost::system::error_code & err, size_t read_bytes) {
			if (read_get_err(err)) return;
			if (header[0] == TCPHEAD1 && header[1] == TCPHEAD2 && header[2] == LOGHEAD1 && header[3] == LOGHEAD2 && read_bytes == HEAD_SIZE) {
				uint32_t pack_size = get_num_from_buf(&header[4]);
				boost::shared_array<char> pDataBuff = boost::shared_array<char>(new char[pack_size]);
				boost::asio::async_read(sock, boost::asio::buffer(pDataBuff.get(), pack_size), boost::asio::transfer_exactly(pack_size),
					std::bind(&LogConnection::on_read_log, this, pDataBuff, std::placeholders::_1, std::placeholders::_2));
				return;
			}
			stopflag = true;
			YT_DEBUG_PRINTF("read failed : recv an invalid header : %c %c %c %d %d\n",
				header[0], header[1], header[2], header[3], get_num_from_buf(&header[4]));
			err_CallBack(remote_ep);
			return;
		}
		void on_read_log(boost::shared_array<char>& buff_, const boost::system::error_code & err, size_t read_bytes) {
			if (read_get_err(err)) return;
			m_logQueue.Enqueue(sharedBuf(buff_, static_cast<uint32_t>(read_bytes)));
			do_read_head();
		}
		//此方法里还需判断格式是否正确
		void logHandel() {
			std::ofstream lgfile;
			while (!stopflag) {
				sharedBuf buff_;
				if (m_logQueue.BlockDequeue(buff_)) {
					if (buff_.buf_size < 10) return;
					//日志等级
					uint8_t lglvl = static_cast<uint8_t>(buff_.buf[0]);

					//提取该条日志的时间
					uint64_t &&logTime = get_num_from_buf_64bit(&(buff_.buf[1]));
					boost::posix_time::ptime pt;
					memcpy(&pt, &logTime, 8);
					std::string slogTime = boost::posix_time::to_iso_string(pt);

					uint32_t msgpos = 9;
					//检查是否是此连接第一条日志
					if (m_bFirstLogFlag) {
						if (buff_.buf_size < 14) return;
						//建立文件夹
						uint32_t clientID = get_num_from_buf(&buff_.buf[9]);
						std::string dbname(std::to_string(clientID) + '(' + sock.remote_endpoint().address().to_string() + ')');
						tpath curLogPath = (*plogPath) / dbname;
						boost::filesystem::create_directories(curLogPath);

						//新建数据库
						curLogPath = curLogPath / (dbname + '_' + slogTime + ".txt");
						lgfile.open(curLogPath.c_str(), std::ios::trunc);
						msgpos += 4;
						m_bFirstLogFlag = false;
					}

					//将日志写入日志文件
					lgfile << '[' << pt << "]<" << (boost::log::trivial::severity_level)(lglvl) << ">:" << std::string(&(buff_.buf[msgpos]), buff_.buf_size - msgpos) << std::endl;
				}
			}	
			if(!m_bFirstLogFlag) lgfile.close();
		}

		QueueBase<sharedBuf> m_logQueue;
		std::thread m_handleThread;
		tpath const *plogPath;
		bool m_bFirstLogFlag;
	};
	

	class LoggerServer : public TcpConnectionPool<LogConnection>{
	public:
		LoggerServer(uint16_t port_,const tstring& path_= T_TEXT(""), uint32_t threadSize_ = 10):
			TcpConnectionPool(port_, threadSize_), logPath(tGetAbsolutePath(path_)){
			boost::filesystem::create_directories(logPath);
		}
		virtual ~LoggerServer() {}
		
	private:
		TcpConnectionPtr getNewTcpConnectionPtr() {
			return std::make_shared<LogConnection>(service, std::bind(&LoggerServer::on_err, this, std::placeholders::_1), &logPath);
		}
		void on_err(const TcpEp& ep){
			TcpConnectionPool::on_err(ep);
		}
		const tpath logPath;
		
	};
	
}