/**
 * @file tcp_conn_pool.hpp
 * @brief TCP连接池
 * @details 基于boost.asio的tcp连接池
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "net_util.hpp"
#include "tcp_conn.hpp"

#include <functional>
#include <map>

namespace ytlib {

/**
 * @brief tcp连接池基类
 * 包头和包内容应该尽可能合在一个tcp包中，否则各层协议前缀消耗很大
 * 单线程+协程无锁模型，io_context::strand由外层传入
 */
class ConnPoolBase {
 public:
  using Strand = boost::asio::io_context::strand;

 public:
  // sock管理结构，不要放逻辑在里面
  class Conn {
   public:
    Conn(Strand& strand) : sock_(strand), strand_(strand) {}
    ~Conn() {}

    // no copy
    Conn(const Conn&) = delete;
    Conn& Conn = (const Conn&) = delete;

    TcpSocket sock_;  //sock连接
    Strand& strand_;
  };

 public:
  ConnPoolBase(Strand& strand, uint16_t port) : port_(port),
                                                strand_(strand),
                                                acceptor_(strand, TcpEp(boost::asio::ip::tcp::v4(), port)) {}
  virtual ~ConnPoolBase() {}

  // no copy
  ConnPoolBase(const ConnPoolBase&) = delete;
  ConnPoolBase& ConnPoolBase = (const ConnPoolBase&) = delete;

  bool Start() {
    if (!CheckPort(port_)) return false;

    boost::asio::spawn(strand_, [this](boost::asio::yield_context yield) {
      while (true) {
        std::shared_ptr<Conn> conn_ptr = std::make_shared<Conn>(strand_);

        boost::system::error_code ec;
        acceptor_.async_accept(conn_ptr->sock_, yield[ec]);

        if (ec) {
          DBG_PRINT("acceptor get err, err: %s", ec.message().c_str());
        } else {
          const TcpEp& ep = conn_ptr->sock_.remote_endpoint();
          DBG_PRINT("get a new connection from %s:%d", ep.address().to_string().c_str(), ep.port());

          conn_map_[ep] = conn_ptr;

          StartConn(conn_ptr);
        }
      }
    });

    return true;
  }

 protected:
  void StartConn(std::shared_ptr<Conn> conn_ptr) {
    boost::asio::spawn(strand_, [this](boost::asio::yield_context yield) {

    });
  }

 protected:
  const uint16_t port_;  //监听端口

  Strand& strand_;
  boost::asio::ip::tcp::acceptor acceptor_;            //监听器
  std::map<IPAddr, std::shared_ptr<Conn> > conn_map_;  //目标ep-ConnBase的map
};

}  // namespace ytlib
