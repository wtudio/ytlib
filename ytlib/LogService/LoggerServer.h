#pragma once
#include <ytlib/Common/Util.h>
#include <ytlib/Common/FileSystem.h>
#include <ytlib/NetTools/TcpConnectionPool.h>
#include <sqlite/sqlite3.h>
#include <boost/shared_array.hpp>
#include <ytlib/SupportTools/ChannelBase.h>

namespace ytlib {
	/*
	目前版本日志服务器以目标机器id+ip:port为标签建立table
	日志内容只有time、level、msg
	初始化时给定一个路径，日志服务器每新连接一个客户机就在此目录下建立id_ip_port路径
	并以time_ip_id.db的名称建立日志数据库。time为日志文件建立的时间
	当满足一定条件（达到某个时间点、日志文件大小过大）时新建一个数据库
	*/

	
	//创建表的sql语句
	static const char * create_table_sql =
		"create table if not exists LOG (\
			id INTEGER not null PRIMARY KEY AUTOINCREMENT,\
			time TEXT not null,\
			level INTEGER not null,\
			msg TEXT not null\
		);";
	static const char * insert_log_sql = "INSERT INTO LOG(time,level,msg) values(?,?,?)";
	static const uint32_t insert_log_sql_size = static_cast<uint32_t>(strlen(insert_log_sql));
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
			do_read_head();
			m_logQueue.Enqueue(shared_buf(buff_, static_cast<uint32_t>(read_bytes)));
		}
		//解析存储日志。格式应该为：4byte客户端id+1byte日志等级+14byte时间信息+n byte日志信息
		//此方法里还需判断格式是否正确
		void logHandel() {
			sqlite3 *db;
			while (!stopflag) {
				shared_buf buff_;
				if (m_logQueue.BlockDequeue(buff_)) {
					if (buff_.buf_size < 20) return;
					//检查是否是此连接第一条日志
					if (m_bFirstLogFlag) {
						//建立文件夹
						uint32_t clientID = get_num_from_buf(buff_.buf.get());
						std::string dbname(std::to_string(clientID) + '(' + sock.remote_endpoint().address().to_string() + ')');
						tpath curLogPath = (*plogPath) / dbname;
						boost::filesystem::create_directories(curLogPath);
						//提取该条日志的时间：20170809114750 14字节
						dbname = dbname + '_' + std::string(&(buff_.buf[5]), 14) + ".db";
						//新建数据库
						uint32_t rc;
						rc = sqlite3_open((curLogPath / dbname).string().c_str(), &db);
						if (rc) {
							YT_DEBUG_PRINTF("open log database failed: %s\n", sqlite3_errmsg(db));
							return;
						}
						char *zErr;
						rc = sqlite3_exec(db, create_table_sql, NULL, NULL, &zErr);
						if (rc) {
							sqlite3_close(db);
							YT_DEBUG_PRINTF("err in create table: %s\n", zErr);
							sqlite3_free(zErr);
							return;
						}
						m_bFirstLogFlag = false;
					}
					//解析日志
					uint8_t lglvl = static_cast<uint8_t>(buff_.buf[4]);
					//将日志存入数据库。日志
					sqlite3_stmt *stmt;
					sqlite3_prepare_v2(db, insert_log_sql, insert_log_sql_size, &stmt, NULL);
					sqlite3_bind_text(stmt, 1, &(buff_.buf[5]), 14, SQLITE_STATIC);
					sqlite3_bind_int(stmt, 2, lglvl);
					sqlite3_bind_text(stmt, 3, &(buff_.buf[19]), buff_.buf_size - 19, SQLITE_STATIC);

					sqlite3_step(stmt);
					sqlite3_finalize(stmt);
					
				}
			}	
			if(!m_bFirstLogFlag) sqlite3_close(db);
		}

		QueueBase<shared_buf> m_logQueue;
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
			return TcpConnectionPtr(new LogConnection(service, std::bind(&LoggerServer::on_err, this, std::placeholders::_1),&logPath));
		}
		void on_err(const TcpEp& ep){
			TcpConnectionPool::on_err(ep);
		}
		const tpath logPath;
		
	};
	
}