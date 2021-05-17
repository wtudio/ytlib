/**
 * @file TcpConnectionPool.h
 * @brief TCP连接池
 * @details 基于boost.asio的tcp连接类和连接池
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/Common/Util.h>
#include <ytlib/NetTools/TcpNetUtil.h>

#include <boost/thread.hpp>
#include <shared_mutex>

namespace ytlib {
/**
 * @brief 网络连接基类
 * 管理单个sock连接。提供一个默认的基类，子类需要重载一些函数以实现具体功能
 */
class ConnectionBase : boost::noncopyable {
 public:
  enum {
    TCPHEAD1 = 'Y',
    TCPHEAD2 = 'T',
    TCPEND1 = 'O',
    TCPEND2 = 'V'
  };
  static const uint8_t HEAD_SIZE = 8;
  ConnectionBase(boost::asio::io_service& io_, std::function<void(const TcpEp&)> errcb_) : sock(io_), err_CallBack(errcb_), stopflag(false) {
  }
  virtual ~ConnectionBase() {
    stopflag = true;
  }
  virtual void start() { do_read_head(); }

  TcpSocket sock;
  TcpEp remote_ep;

 protected:
  virtual void do_read_head() {
    boost::asio::async_read(sock, boost::asio::buffer(header, HEAD_SIZE), boost::asio::transfer_exactly(HEAD_SIZE),
                            std::bind(&ConnectionBase::on_read_head, this, std::placeholders::_1, std::placeholders::_2));
  }
  bool read_get_err(const boost::system::error_code& err) {
    if (stopflag) return true;
    if (err) {
      stopflag = true;
      YT_DEBUG_PRINTF("read failed : %s", err.message().c_str());
      err_CallBack(remote_ep);
      return true;
    }
    return false;
  }
  virtual void on_read_head(const boost::system::error_code& err, std::size_t read_bytes) {
    if (read_get_err(err)) return;
    if (header[0] == TCPHEAD1 && header[1] == TCPHEAD2 && read_bytes == HEAD_SIZE) {
      uint32_t pack_size = get_num_from_buf(&header[4]);
      //do something
      return;
    }
    stopflag = true;
    YT_DEBUG_PRINTF("read failed : recv an invalid header : %c %c %c %c %d",
                    header[0], header[1], header[2], header[3], get_num_from_buf(&header[4]));
    err_CallBack(remote_ep);
    return;
  }
  std::atomic_bool stopflag;
  std::function<void(const TcpEp&)> err_CallBack;  ///<发生错误时的回调。一旦读/写出错，就关闭连接并调用回调告知上层
  char header[HEAD_SIZE];                          ///<接收缓存
};

/**
 * @brief tcp连接池基类
 * 需要继承重载getNewTcpConnectionPtr才能使用。只能用来被动监听连接
 */
template <class T_Connection>
class TcpConnectionPool {
 protected:
  typedef std::shared_ptr<T_Connection> TcpConnectionPtr;

  std::atomic_bool stopflag;  //停止标志
  boost::thread_group m_RunThreads;

  std::map<TcpEp, TcpConnectionPtr> m_mapTcpConnection;  //目标ep-TcpConnection的map
  std::shared_mutex m_TcpConnectionMutex;

  std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptorPtr;  //监听器
  boost::asio::io_service service;

  const uint32_t m_threadSize;  //使用的异步线程数量
  const uint16_t myport;        //监听端口，并且所有主动进行的连接都绑定到这个端口上

  virtual TcpConnectionPtr getNewTcpConnectionPtr() {
    return TcpConnectionPtr();
  }
  virtual void on_accept(TcpConnectionPtr& p, const boost::system::error_code& err) {
    if (stopflag) return;
    if (err) {
      YT_DEBUG_PRINTF("listerner get err, please restart : %s", err.message().c_str());
      return;
    }
    p->remote_ep = p->sock.remote_endpoint();
    YT_DEBUG_PRINTF("get a new connection from %s:%d", p->remote_ep.address().to_string().c_str(), p->remote_ep.port());
    m_TcpConnectionMutex.lock();
    m_mapTcpConnection[p->remote_ep] = p;
    m_TcpConnectionMutex.unlock();
    p->start();
    TcpConnectionPtr pConnection = getNewTcpConnectionPtr();
    if (!pConnection) {
      stop();
      return;
    }
    acceptorPtr->async_accept(pConnection->sock, std::bind(&TcpConnectionPool::on_accept, this, pConnection, std::placeholders::_1));
  }
  virtual void on_err(const TcpEp& ep) {
    YT_DEBUG_PRINTF("connection to %s:%d get an err and is closed", ep.address().to_string().c_str(), ep.port());
    std::unique_lock<std::shared_mutex> lck(m_TcpConnectionMutex);
    typename std::map<TcpEp, TcpConnectionPtr>::iterator itr = m_mapTcpConnection.find(ep);
    if (itr != m_mapTcpConnection.end()) {
      m_mapTcpConnection.erase(itr);
    }
  }

 public:
  TcpConnectionPool(uint16_t port_,
                    uint32_t threadSize_ = 10) : myport(port_),
                                                 m_threadSize(threadSize_),
                                                 stopflag(true) {
  }
  virtual ~TcpConnectionPool() {
    stop();
  }
  virtual bool start() {
    if (!checkPort(myport)) return false;
    //如果要做高并发连接的话可以在此处添加异步accept的个数
    TcpConnectionPtr pConnection = getNewTcpConnectionPtr();
    if (!pConnection) return false;
    service.reset();
    stopflag = false;
    acceptorPtr = std::make_shared<boost::asio::ip::tcp::acceptor>(service, TcpEp(boost::asio::ip::tcp::v4(), myport), true);
    acceptorPtr->async_accept(pConnection->sock, std::bind(&TcpConnectionPool::on_accept, this, pConnection, std::placeholders::_1));
    for (uint32_t i = 0; i < m_threadSize; ++i) {
      m_RunThreads.create_thread(boost::bind(&boost::asio::io_service::run, &service));
    }
    return true;
  }
  virtual void stop() {
    if (!stopflag) {
      stopflag = true;
      service.stop();
      acceptorPtr.reset();
      std::unique_lock<std::shared_mutex> lck(m_TcpConnectionMutex);
      m_mapTcpConnection.clear();
      lck.unlock();
      m_RunThreads.join_all();
    }
  }
};

}  // namespace ytlib