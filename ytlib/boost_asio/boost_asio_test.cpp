#include <gtest/gtest.h>

#include "net_util.hpp"

namespace ytlib {

using namespace boost::asio;
using namespace std;

TEST(BOOST_ASIO_TEST, UTIL) {
  uint16_t port = GetUsablePort();
  ASSERT_TRUE(CheckPort(port));

  char buf[4];
  uint32_t n = 123456789;
  SetBufFromNum(buf, n);
  ASSERT_EQ(GetNumFromBuf(buf), n);
}

}  // namespace ytlib
