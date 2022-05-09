#include <gtest/gtest.h>

#include "net_util.hpp"

namespace ytlib {

using namespace boost::asio;
using namespace std;

TEST(BOOST_ASIO_TEST, UTIL) {
  uint16_t port = GetUsablePort();
  ASSERT_TRUE(CheckPort(port));

  {
    char buf[4];
    uint32_t n = 123456789;
    SetBufFromUint32(buf, n);
    ASSERT_EQ(GetUint32FromBuf(buf), n);
  }

  {
    char buf[2];
    uint32_t n = 56789;
    SetBufFromUint16(buf, n);
    ASSERT_EQ(GetUint16FromBuf(buf), n);
  }
}

}  // namespace ytlib
