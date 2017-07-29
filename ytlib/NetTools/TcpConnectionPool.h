#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <shared_mutex>
#include <atomic>

namespace ytlib {

	typedef boost::asio::ip::tcp::endpoint TcpEp;//28个字节

	//sock连接，为网络适配器定制准备，只能主动发起同步写，读取是异步自动的
	class TcpConnectionBase  :boost::noncopyable {
	public:
		TcpConnectionBase(boost::asio::io_service& io_, std::function<void(const TcpEp &)> errcb_) :
			sock(io_), err_CallBack(errcb_), stopflag(false){}
		virtual ~TcpConnectionBase() {
			stopflag = true;
		}
		//用于主动连接
		bool connect(const TcpEp& ep_,uint16_t port_ = 0) {
			sock.open(boost::asio::ip::tcp::v4());
			if (port_) {
				sock.set_option(boost::asio::ip::tcp::socket::reuse_address(true));
				sock.bind(TcpEp(boost::asio::ip::tcp::v4(), port_));//如果指定端口了，则绑定到指定端口
			}
			boost::system::error_code err;
			sock.connect(ep_, err);
			if (err) {
				printf_s("connect failed : %s\n", err.message().c_str());
				return false;
			}
			remote_ep = ep_;
			return true;
		}
		//同步写
		virtual bool write(const std::string & data) {
			boost::system::error_code err;
			sock.write_some(boost::asio::buffer(data), err);
			if (err) {
				if (stopflag) return false;
				printf_s("write failed : %s\n", err.message().c_str());
				err_CallBack(remote_ep);
				return false;
			}
			return true;
		}
		//已经连接了，开启异步read
		virtual void start() {
			std::shared_ptr<boost::asio::streambuf> pbuf = std::shared_ptr<boost::asio::streambuf>(new boost::asio::streambuf);
			boost::asio::async_read(sock, *pbuf, std::bind(&TcpConnectionBase::on_read, this, pbuf, std::placeholders::_1, std::placeholders::_2));
		}

		virtual void on_read(std::shared_ptr<boost::asio::streambuf>& pbuf, const boost::system::error_code & err, size_t read_bytes) {
			if (stopflag) return;
			if (err) {
				printf_s("read failed : %s\n", err.message().c_str());
				err_CallBack(remote_ep);
				return;
			}
			boost::asio::streambuf::const_buffers_type cbt = pbuf->data();
			std::string msg(boost::asio::buffers_begin(cbt), boost::asio::buffers_end(cbt));
			printf_s("read success : %s\n", msg.c_str());
			pbuf->consume(pbuf->size());
			boost::asio::async_read(sock, *pbuf, std::bind(&TcpConnectionBase::on_read, this, pbuf, std::placeholders::_1, std::placeholders::_2));
		}
		
		boost::asio::ip::tcp::socket sock;
		TcpEp remote_ep;
		std::function<void(const TcpEp &)> err_CallBack;//发生错误时的回调。一旦读/写出错，就关闭连接并调用回调告知上层
		std::atomic_bool stopflag;
	};


	/*
	sock连接池,	两种方式建立连接：
	1、accep到的
	2、主动连接的
	本地的连接都绑定到一个ep上
	*/
	template<class T_TcpConnection>//TcpConnection
	class TcpConnectionPoolBase {
	public:
		typedef std::shared_ptr<T_TcpConnection> TcpConnectionPtr;
		//检查端口是否被占用。如果true代表未被占用
		static bool checkPort(uint16_t port_) {
			boost::asio::io_service io;
			boost::asio::ip::tcp::socket sk(io);
			sk.open(boost::asio::ip::tcp::v4());
			boost::system::error_code err;
			sk.connect(TcpEp(boost::asio::ip::tcp::v4(), port_), err);
			if (err) return true;//连接不上，说明没有程序在监听
			sk.close();//否则说明已经被占用了
			return false;
		}

		explicit TcpConnectionPoolBase(uint16_t port_) :myport(port_), stopflag(false){}
		virtual ~TcpConnectionPoolBase(){ 
			stopflag = true;
			service.stop();
			acceptorPtr.reset();
			m_mapTcpConnection.swap(std::map<TcpEp, TcpConnectionPtr>());
			m_RunThreads.join_all();
		}
		//开启监听
		bool start() {
			if (stopflag || !TcpConnectionPoolBase::checkPort(myport)) return false;
			service.reset();
			acceptorPtr = std::shared_ptr<boost::asio::ip::tcp::acceptor>(
				new boost::asio::ip::tcp::acceptor(service, TcpEp(boost::asio::ip::tcp::v4(), myport), true));
			TcpConnectionPtr pConnection = getNewTcpConnectionPtr();
			acceptorPtr->async_accept(pConnection->sock, std::bind(&TcpConnectionPoolBase::on_accept, this, pConnection, std::placeholders::_1));
			for (int i = 0; i < 10; i++) {
				m_RunThreads.create_thread(boost::bind(&boost::asio::io_service::run, &service));
			}
			return true;
		}
		//获取连接
		TcpConnectionPtr getTcpConnectionPtr(const TcpEp& ep) {
			if (stopflag) return TcpConnectionPtr();
			//先在map里找，没有就直接去连接一个
			{
				std::shared_lock<std::shared_mutex> lck(m_TcpConnectionMutex);
				std::map<TcpEp, TcpConnectionPtr>::iterator itr = m_mapTcpConnection.find(ep);
				if (itr != m_mapTcpConnection.end()) {
					return itr->second;
				}
			}
			//同步连接
			TcpConnectionPtr pConnection = getNewTcpConnectionPtr();
			if (pConnection->connect(ep, myport)) {
				printf_s("connect to %s:%d successful\n", ep.address().to_string().c_str(), ep.port());
				pConnection->start();
				std::unique_lock<std::shared_mutex> lck(m_TcpConnectionMutex);
				m_mapTcpConnection[ep] = pConnection;
				return pConnection;
			}
			else{
				//如果同步连接失败了，也有可能对方已经连接过来了。可以再在表中找一下
				std::shared_lock<std::shared_mutex> lck(m_TcpConnectionMutex);
				std::map<TcpEp, TcpConnectionPtr>::iterator itr = m_mapTcpConnection.find(ep);
				if (itr != m_mapTcpConnection.end()) {
					return itr->second;
				}
			}
			printf_s("connect to %s:%d failed\n", ep.address().to_string().c_str(), ep.port());
			return TcpConnectionPtr();
		}
		//需要将其清除
		void on_err(const TcpEp& ep) {
			printf_s("connection to %s:%d get an err and is closed\n", ep.address().to_string().c_str(), ep.port());
			std::unique_lock<std::shared_mutex> lck(m_TcpConnectionMutex);
			std::map<TcpEp, TcpConnectionPtr>::iterator itr = m_mapTcpConnection.find(ep);
			if (itr != m_mapTcpConnection.end()) {
				m_mapTcpConnection.erase(itr);
			}
		}
		//工厂函数
		virtual TcpConnectionPtr getNewTcpConnectionPtr() = 0;

		void on_accept(TcpConnectionPtr& p, const boost::system::error_code & err) {
			if (stopflag) return;
			if (err) {
				printf_s("listerner get err, please restart : %s\n", err.message().c_str());
				return;
			}
			p->remote_ep = p->sock.remote_endpoint();
			printf_s("get a new connection from %s:%d\n", p->remote_ep.address().to_string().c_str(), p->remote_ep.port());
			p->start();
			std::unique_lock<std::shared_mutex> lck(m_TcpConnectionMutex);
			m_mapTcpConnection[p->remote_ep] = p;
			TcpConnectionPtr pConnection = getNewTcpConnectionPtr();
			acceptorPtr->async_accept(pConnection->sock, std::bind(&TcpConnectionPoolBase::on_accept, this, pConnection, std::placeholders::_1));

		}
		const uint16_t myport;//监听端口，并且所有主动进行的连接都绑定到这个端口上
		std::map<TcpEp, TcpConnectionPtr> m_mapTcpConnection;//目标ep-TcpConnection的map
		std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptorPtr;//监听器
		std::shared_mutex m_TcpConnectionMutex;
		boost::thread_group m_RunThreads;
		boost::asio::io_service service;
		std::atomic_bool stopflag;
	};


}