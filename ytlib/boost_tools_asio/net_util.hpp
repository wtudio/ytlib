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

inline std::string TcpEp2Str(const boost::asio::ip::tcp::endpoint& ep) {
  std::stringstream ss;
  ss << ep;
  return ss.str();
}

///检查端口是否可用。true说明可用
inline bool CheckPort(uint16_t port_) {
  boost::asio::io_context io;
  boost::asio::ip::tcp::socket sk(io);
  sk.open(boost::asio::ip::tcp::v4());
  boost::system::error_code err;
  sk.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_), err);
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

///大小端转换，将ps中的数据转换到pd中。默认小端
inline void TransEndian(char* pd, const char* ps, uint32_t len) {
  static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "unknown endian");
  if constexpr (std::endian::native == std::endian::big) {
    ps += len;
    while (len--) (*(pd++)) = (*(--ps));
  } else if constexpr (std::endian::native == std::endian::little) {
    memcpy(pd, ps, len);
  }
}

inline void SetBufFromUint32(char* p, uint32_t n) {
  static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "unknown endian");
  if constexpr (std::endian::native == std::endian::big) {
    p[0] = ((char*)&n)[3];
    p[1] = ((char*)&n)[2];
    p[2] = ((char*)&n)[1];
    p[3] = ((char*)&n)[0];
  } else if constexpr (std::endian::native == std::endian::little) {
    memcpy(p, &n, 4);
  }
}

inline uint32_t GetUint32FromBuf(const char* p) {
  static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "unknown endian");
  if constexpr (std::endian::native == std::endian::big) {
    uint32_t n;
    ((char*)&n)[3] = p[0];
    ((char*)&n)[2] = p[1];
    ((char*)&n)[1] = p[2];
    ((char*)&n)[0] = p[3];
    return n;
  } else if constexpr (std::endian::native == std::endian::little) {
    return *((uint32_t*)p);
  }
}

inline void SetBufFromUint16(char* p, uint16_t n) {
  static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "unknown endian");
  if constexpr (std::endian::native == std::endian::big) {
    p[0] = ((char*)&n)[1];
    p[1] = ((char*)&n)[0];
  } else if constexpr (std::endian::native == std::endian::little) {
    memcpy(p, &n, 2);
  }
}

inline uint16_t GetUint16FromBuf(const char* p) {
  static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "unknown endian");
  if constexpr (std::endian::native == std::endian::big) {
    uint16_t n;
    ((char*)&n)[1] = p[0];
    ((char*)&n)[0] = p[1];
    return n;
  } else if constexpr (std::endian::native == std::endian::little) {
    return *((uint16_t*)p);
  }
}

}  // namespace ytlib
