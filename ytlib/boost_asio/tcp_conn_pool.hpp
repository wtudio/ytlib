/**
 * @file tcp_conn_pool.hpp
 * @brief TCP连接池
 * @details 基于boost.asio的tcp连接池
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "net_util.hpp"

#include <functional>
#include <list>
#include <map>
#include <shared_mutex>
#include <thread>

namespace ytlib {

class ConnBase {
 public:
};

/**
 * @brief tcp连接池基类
 * 只有一个线程接收
 * 用vector<char>做输出buf
 * 包头和包内容应该尽可能合在一个tcp包中，否则各层协议前缀消耗很大
 */
class ConnPoolBase {
 public:
  using RecvCheckFun = std::function<size_t(const char* buf, size_t len)>;
  using RecvHandleFun = std::function<void(const char* buf, size_t len)>;

 public:
  ConnPoolBase() {}
  virtual ~ConnPoolBase() {}

 protected:
  std::atomic_bool stopflag_;       //停止标志
  std::list<std::thread> threads_;  //线程

  std::map<IPAddr, ConnBase*> tcp_conn_map_;  //目标ep-TcpConnection的map
  std::shared_mutex tcp_conn_map_mutex_;

  boost::asio::ip::tcp::acceptor* acceptor_ptr_;  //监听器
  boost::asio::io_service service_;

  const uint32_t thread_size_;  //使用的异步线程数量
  const uint16_t port_;         //监听端口，并且所有主动进行的连接都绑定到这个端口上
};

}  // namespace ytlib
