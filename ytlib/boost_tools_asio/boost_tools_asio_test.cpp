#include <gtest/gtest.h>

#include "asio_tools.hpp"
#include "net_util.hpp"

namespace ytlib {

using namespace boost::asio;
using namespace std;

TEST(BOOST_TOOLS_ASIO_TEST, UTIL) {
  uint16_t port = GetUsablePort();
  EXPECT_TRUE(CheckPort(port));

  {
    char buf[4];
    uint32_t n = 123456789;
    SetBufFromUint32(buf, n);
    EXPECT_EQ(GetUint32FromBuf(buf), n);
  }

  {
    char buf[2];
    uint32_t n = 56789;
    SetBufFromUint16(buf, n);
    EXPECT_EQ(GetUint16FromBuf(buf), n);
  }
}

TEST(BOOST_TOOLS_ASIO_TEST, AsioExecutor) {
  auto asio_sys_ptr = std::make_shared<AsioExecutor>(2);
  EXPECT_EQ(asio_sys_ptr->ThreadsNum(), 2);

  std::thread t([asio_sys_ptr] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    asio_sys_ptr->Stop();
  });

  asio_sys_ptr->Start();
  asio_sys_ptr->Join();

  t.join();
}

}  // namespace ytlib
