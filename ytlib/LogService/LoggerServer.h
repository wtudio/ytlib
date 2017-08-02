#pragma once
#include <ytlib/Common/Util.h>
#include <ytlib/NetTools/TcpConnectionPool.h>
#include <sqlite/sqlite3.h>
#include <boost/shared_array.hpp>


//todo： 使用boost.log库、网络适配器、简单数据库来完成一个日志服务器

namespace ytlib {

	//连接类。tag始终为 LG
	class LogConnection :public ConnectionBase {
	public:
		enum {
			LOGHEAD1 = 'L',
			LOGHEAD2 = 'G'
		};
		
		LogConnection(boost::asio::io_service& io_, std::function<void(const TcpEp &)> errcb_) :
			ConnectionBase(io_, errcb_){

		}
		virtual ~LogConnection() {stopflag = true;}

	private:
		void do_read_head() {
			boost::asio::async_read(sock, boost::asio::buffer(header, HEAD_SIZE), boost::asio::transfer_exactly(HEAD_SIZE),
				std::bind(&LogConnection::on_read_head, this, std::placeholders::_1, std::placeholders::_2));
		}
		//读取解析报头
		void on_read_head(const boost::system::error_code & err, size_t read_bytes) {
			if (read_get_err(err)) return;
			if (header[0] == TCPHEAD1 && header[1] == TCPHEAD2 &&
				header[2] == LOGHEAD1 && header[3] == LOGHEAD2 && read_bytes == HEAD_SIZE) {
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
			do_read_head();
			//解析存储日志

		}

	};

	class LoggerServer : public TcpConnectionPool<LogConnection>{
	public:
		LoggerServer(uint16_t port_,uint32_t threadSize_ = 10):TcpConnectionPool(port_, threadSize_){

		}
		virtual ~LoggerServer() {}
		
	private:
		TcpConnectionPtr getNewTcpConnectionPtr() {
			return TcpConnectionPtr(new LogConnection(service, std::bind(&LoggerServer::on_err, this, std::placeholders::_1)));
		}
		void on_err(const TcpEp& ep){
			TcpConnectionPool::on_err(ep);
		}

	};
	
}