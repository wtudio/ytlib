/**
 * @file tcp_conn.hpp
 * @brief TCP连接类
 * @details 基于boost.asio的tcp连接
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "net_util.hpp"
#include "ytlib/misc/misc_macro.hpp"

#include <list>
#include <shared_mutex>
#include <thread>

namespace ytlib {

/**
 * @brief 网络连接类接口
 */
class ConnBase {
 public:
  ConnBase() {}
  virtual ~ConnBase() {}

  // no copy
  ConnBase(const ConnBase&) = delete;
  ConnBase& ConnBase = (const ConnBase&) = delete;

  virtual size_t RecvCheckFun(const char* buf, size_t len) = 0;
  virtual bool Connect(const TcpEp& ep) = 0;

 protected:
  TcpSocket sock_;   //sock连接
  TcpEp remote_ep_;  //远端地址
};

/**
 * @brief 网络连接类接口
 * 管理单个sock连接。提供一个默认的基类，子类需要重载一些函数以实现具体功能
 * 标准数据包发送规范：
 * step1：先传一个报头：（8 byte）
 *   head: 2 byte
 *   tag: 2 byte
 *   size: 4 byte ：默认小端传输
 * step2：传输size个byte的数据
 * 如果使用结束符的话，需要在发送完成一个包后发送一个结束head：tag = TCPEND1 + TCPEND2
 */
class DemoConn {
 public:
  enum {
    TCPHEAD1 = 'Y',
    TCPHEAD2 = 'T',
    TCPEND1 = 'O',
    TCPEND2 = 'V'
  };
  static const uint8_t HEAD_SIZE = 8;

  DemoConn(boost::asio::io_service& service,
           std::function<void(const TcpEp&)> errcb) : sock_(service), errcb_(errcb), stopflag_(false) {}
  virtual ~DemoConn() { stopflag_ = true; }

  // no copy
  DemoConn(const DemoConn&) = delete;
  DemoConn& DemoConn = (const DemoConn&) = delete;

  virtual void Start() { ReadHead(); }

  TcpSocket sock_;   //sock连接
  TcpEp remote_ep_;  //远端地址

 protected:
  //异步读head
  virtual void ReadHead() {
    boost::asio::async_read(sock_,
                            boost::asio::buffer(header, HEAD_SIZE),
                            boost::asio::transfer_exactly(HEAD_SIZE),
                            std::bind(&DemoConn::OnReadHead, this, std::placeholders::_1, std::placeholders::_2));
  }

  //示例包头读取回调
  virtual void OnReadHead(const boost::system::error_code& err, std::size_t read_bytes) {
    if (stopflag_) return;

    if (err) {
      stopflag_ = true;
      DBG_PRINT("read failed: %s", err.message().c_str());
      errcb_(remote_ep_);
      return;
    }

    if (!(header[0] == TCPHEAD1 && header[1] == TCPHEAD2 && read_bytes == HEAD_SIZE)) {
      stopflag_ = true;
      DBG_PRINT("read failed: recv an invalid header : %c %c %c %c %d",
                header[0], header[1], header[2], header[3], GetNumFromBuf(&header[4]));
      errcb_(remote_ep_);
      return;
    }

    uint32_t pack_size = GetNumFromBuf(&header[4]);
    //do something
    return;
  }

  std::atomic_bool stopflag_;                ///<停止标志
  std::function<void(const TcpEp&)> errcb_;  ///<发生错误时的回调。一旦读/写出错，就关闭连接并调用回调告知上层
  char header[HEAD_SIZE];                    ///<接收缓存
};

/**
 * @brief tcp连接池基类
 */
class ConnPool {
  typedef std::shared_ptr<ConnBase> TcpConnectionPtr;

 public:
  ConnPool(uint16_t port, uint32_t thread_size = 10) : port_(port), thread_size_(thread_size), stopflag_(true) {}
  virtual ~ConnPool() { Stop(); }

  virtual bool Start() {
    if (!CheckPort(port_)) return false;

    //如果要做高并发连接的话可以在此处添加异步accept的个数
    TcpConnectionPtr conn_ptr = GetNewTcpConnectionPtr();
    if (!conn_ptr) return false;

    service_.reset();
    stopflag_ = false;
    acceptor_ptr_ = std::make_shared<boost::asio::ip::tcp::acceptor>(service_, TcpEp(boost::asio::ip::tcp::v4(), port_), true);
    acceptor_ptr_->async_accept(conn_ptr->sock_, std::bind(&ConnPool::OnAccept, this, conn_ptr, std::placeholders::_1));

    for (uint32_t ii = 0; ii < thread_size_; ++ii) {
      threads_.emplace(threads_.end(), [&service_] {
        service_.run();
      });
    }
    return true;
  }

  virtual void Stop() {
    if (stopflag_) return;

    stopflag_ = true;
    service_.stop();
    acceptor_ptr_.reset();

    tcp_conn_map_mutex_.lock();
    tcp_conn_map_.clear();
    lck.unlock();

    for (auto itr = threads_.begin(); itr != threads_.end();) {
      itr->join();
      threads_.erase(itr++);
    }
  }

 protected:
  virtual TcpConnectionPtr GetNewTcpConnectionPtr() {
    return std::make_shared<ConnBase>(service_, std::bind(&ConnPool::OnErr, this, std::placeholders::_1));
  }

  virtual void OnAccept(TcpConnectionPtr& p, const boost::system::error_code& err) {
    if (stopflag_) return;

    if (err) {
      DBG_PRINT("listerner get err, please restart : %s", err.message().c_str());
      return;
    }

    p->remote_ep_ = p->sock_.remote_endpoint();
    DBG_PRINT("get a new connection from %s:%d", p->remote_ep_.address().to_string().c_str(), p->remote_ep_.port());

    tcp_conn_map_mutex_.lock();
    tcp_conn_map_[p->remote_ep_] = p;
    tcp_conn_map_mutex_.unlock();

    p->Start();

    TcpConnectionPtr conn_ptr = GetNewTcpConnectionPtr();
    if (!conn_ptr) {
      Stop();
      return;
    }
    acceptor_ptr_->async_accept(conn_ptr->sock_, std::bind(&ConnPool::OnAccept, this, conn_ptr, std::placeholders::_1));
  }

  virtual void OnErr(const TcpEp& ep) {
    DBG_PRINT("connection to %s:%d get an err and is closed", ep.address().to_string().c_str(), ep.port());
    std::unique_lock<std::shared_mutex> lck(tcp_conn_map_mutex_);
    auto itr = tcp_conn_map_.find(ep);
    if (itr != tcp_conn_map_.end())
      tcp_conn_map_.erase(itr);
  }

  std::atomic_bool stopflag_;       //停止标志
  std::list<std::thread> threads_;  //线程

  std::map<TcpEp, TcpConnectionPtr> tcp_conn_map_;  //目标ep-TcpConnection的map
  std::shared_mutex tcp_conn_map_mutex_;

  std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr_;  //监听器
  boost::asio::io_service service_;

  const uint32_t thread_size_;  //使用的异步线程数量
  const uint16_t port_;         //监听端口，并且所有主动进行的连接都绑定到这个端口上
};

}  // namespace ytlib