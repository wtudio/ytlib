#include <gtest/gtest.h>

#include "asio_tools.hpp"
#include "asio_udp_cli.hpp"
#include "asio_udp_svr.hpp"

#include "ytlib/boost_tools_util/serialize.hpp"

namespace ytlib {

namespace asio = boost::asio;

TEST(BOOST_TOOLS_ASIO_TEST, UDP_base) {
  AsioDebugTool::Ins().Reset();

  auto cli_sys_ptr = std::make_shared<AsioExecutor>(2);
  auto svr_sys_ptr = std::make_shared<AsioExecutor>(2);

  AsioUdpClient::Cfg udp_cli_cfg;
  udp_cli_cfg.svr_ep = asio::ip::udp::endpoint{asio::ip::address_v4({127, 0, 0, 1}), 53927};
  auto udp_cli_ptr = std::make_shared<AsioUdpClient>(cli_sys_ptr->IO(), udp_cli_cfg);

  cli_sys_ptr->RegisterSvrFunc(std::function<void()>(), [udp_cli_ptr] { udp_cli_ptr->Stop(); });

  std::thread t_cli([cli_sys_ptr] {
    DBG_PRINT("cli_sys_ptr start");
    cli_sys_ptr->Start();
    cli_sys_ptr->Join();
    DBG_PRINT("cli_sys_ptr exit");
  });

  std::string result_str;
  std::thread t_svr([svr_sys_ptr, &result_str] {
    DBG_PRINT("svr_sys_ptr start");
    auto udp_svr_ptr = std::make_shared<AsioUdpServer>(svr_sys_ptr->IO(), AsioUdpServer::Cfg());
    udp_svr_ptr->RegisterMsgHandleFunc(
        [&result_str](const boost::asio::ip::udp::endpoint &ep, const std::shared_ptr<boost::asio::streambuf> &msg_buf_ptr) {
          result_str = std::string(static_cast<const char *>(msg_buf_ptr->data().data()), msg_buf_ptr->size());
          DBG_PRINT("svr get a msg from %s, size: %llu, data: %s", UdpEp2Str(ep).c_str(), msg_buf_ptr->size(), result_str.c_str());
        });

    svr_sys_ptr->RegisterSvrFunc([udp_svr_ptr] { udp_svr_ptr->Start(); },
                                 [udp_svr_ptr] { udp_svr_ptr->Stop(); });

    svr_sys_ptr->Start();
    svr_sys_ptr->Join();
    DBG_PRINT("svr_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    std::shared_ptr<boost::asio::streambuf> msg_buf_ptr = std::make_shared<boost::asio::streambuf>();
    auto buf = msg_buf_ptr->prepare(20);
    sprintf(static_cast<char *>(buf.data()), "1234567890123456789");
    msg_buf_ptr->commit(8);

    auto buf2 = msg_buf_ptr->prepare(20);
    sprintf(static_cast<char *>(buf2.data()), "abcdefghijklmn");
    msg_buf_ptr->commit(8);
    udp_cli_ptr->SendMsg(msg_buf_ptr);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_STREQ(result_str.c_str(), "12345678abcdefgh");

  svr_sys_ptr->Stop();
  t_svr.join();

  cli_sys_ptr->Stop();
  t_cli.join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());
}

}  // namespace ytlib
