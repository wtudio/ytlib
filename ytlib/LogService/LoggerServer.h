#pragma once
#include <ytlib/NetTools/TcpNetUtil.h>
#include <sqlite/sqlite3.h>
#include <boost/thread.hpp>

//todo： 使用boost.log库、网络适配器、简单数据库来完成一个日志服务器

namespace ytlib {

	class LoggerServer {
	private:


		//连接类。tag始终为 LG
		class LogConnection :boost::noncopyable {
		public:
			LogConnection(boost::asio::io_service& io_) :
				sock(io_){

			}


			boost::asio::ip::tcp::socket sock;
		private:

		};
	public:



	};
	
}