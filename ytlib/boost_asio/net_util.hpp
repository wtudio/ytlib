/**
 * @file net_util.hpp
 * @brief 网络基础模块
 * @note 一些网络基础工具，如大小端转换、端口检查等
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <boost/asio.hpp>

namespace ytlib {

typedef boost::asio::ip::address_v4 IPV4;      //ip
typedef boost::asio::ip::tcp::endpoint TcpEp;  //28个字节
typedef boost::asio::ip::tcp::socket TcpSocket;

inline std::string TcpEp2Str(const TcpEp& ep) {
  std::stringstream ss;
  ss << ep;
  return ss.str();
}

///检查端口是否可用。true说明可用
inline bool CheckPort(uint16_t port_) {
  boost::asio::io_context io;
  TcpSocket sk(io);
  sk.open(boost::asio::ip::tcp::v4());
  boost::system::error_code err;
  sk.connect(TcpEp(boost::asio::ip::tcp::v4(), port_), err);
  if (err) return true;  //连接不上，说明没有程序在监听
  sk.close();            //否则说明已经被占用了
  return false;
}

///获取一个未被占用的端口号，如果没有找到则返回0
inline uint16_t GetUsablePort(uint16_t start = 60000, uint16_t end = 65535) {
  for (uint16_t ii = start; ii < end; ++ii) {
    if (CheckPort(ii)) return ii;
  }
  return 0;
}

// 大小端判断
#define ENDIAN_ORDER ('ABCD')
#define LITTLE_ENDIAN 0x41424344UL
#define BIG_ENDIAN 0x44434241UL

///大小端转换，将ps中的数据转换到pd中。默认小端
inline void TransEndian(char* pd, const char* ps, uint32_t len) {
#if ENDIAN_ORDER == LITTLE_ENDIAN
  memcpy(pd, ps, len);
#elif ENDIAN_ORDER == BIG_ENDIAN
  ps += len;
  while (len--) (*(pd++)) = (*(--ps));
#else
  #error "unknown ENDIAN."
#endif
}

inline void SetBufFromNum(char* p, uint32_t n) {
#if ENDIAN_ORDER == LITTLE_ENDIAN
  memcpy(p, &n, 4);
#elif ENDIAN_ORDER == BIG_ENDIAN
  p[0] = ((char*)&n)[3];
  p[1] = ((char*)&n)[2];
  p[2] = ((char*)&n)[1];
  p[3] = ((char*)&n)[0];
#else
  #error "unknown ENDIAN."
#endif
}

inline uint32_t GetNumFromBuf(const char* p) {
#if ENDIAN_ORDER == LITTLE_ENDIAN
  return *((uint32_t*)p);
#elif ENDIAN_ORDER == BIG_ENDIAN
  uint32_t n;
  ((char*)&n)[3] = p[0];
  ((char*)&n)[2] = p[1];
  ((char*)&n)[1] = p[2];
  ((char*)&n)[0] = p[3];
  return n;
#else
  #error "unknown ENDIAN."
#endif
}

}  // namespace ytlib