#include <gtest/gtest.h>

#include "asio_cs_cli.hpp"
#include "asio_cs_svr.hpp"
#include "asio_tools.hpp"

#include "ytlib/boost_tools/serialize.hpp"

namespace ytlib {

namespace asio = boost::asio;

TEST(BOOST_ASIO_TEST, CS_base) {
  AsioDebugTool::Ins().Reset();

  auto cli_sys_ptr = std::make_shared<AsioExecutor>(2);
  auto svr_sys_ptr = std::make_shared<AsioExecutor>(2);

  AsioCsClient::Cfg cs_cli_cfg;
  cs_cli_cfg.svr_ep = asio::ip::tcp::endpoint{asio::ip::address_v4({127, 0, 0, 1}), 57634};
  auto cs_cli_ptr = std::make_shared<AsioCsClient>(cli_sys_ptr->IO(), cs_cli_cfg);
  cs_cli_ptr->RegisterMsgHandleFunc(
      [cs_cli_ptr](std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
        std::string tmp_str(static_cast<const char *>(msg_buf_ptr->data().data()), msg_buf_ptr->size());
        DBG_PRINT("cli get a msg, size: %llu, data: %s", tmp_str.size(), tmp_str.c_str());
      });

  cli_sys_ptr->RegisterSvrFunc(std::function<void()>(), [cs_cli_ptr] { cs_cli_ptr->Stop(); });

  std::thread t_cli([cli_sys_ptr] {
    DBG_PRINT("cli_sys_ptr start");
    cli_sys_ptr->Start();
    cli_sys_ptr->Join();
    DBG_PRINT("cli_sys_ptr exit");
  });

  std::thread t_svr([svr_sys_ptr] {
    DBG_PRINT("svr_sys_ptr start");
    auto cs_svr_ptr = std::make_shared<AsioCsServer>(svr_sys_ptr->IO(), AsioCsServer::Cfg());
    cs_svr_ptr->RegisterMsgHandleFunc(
        [cs_svr_ptr](boost::asio::ip::tcp::endpoint ep, std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
          std::string tmp_str(static_cast<const char *>(msg_buf_ptr->data().data()), msg_buf_ptr->size());
          DBG_PRINT("svr get a msg from %s, size: %llu, data: %s", TcpEp2Str(ep).c_str(), tmp_str.size(), tmp_str.c_str());
          cs_svr_ptr->SendMsg(ep, msg_buf_ptr);
        });

    svr_sys_ptr->RegisterSvrFunc([cs_svr_ptr] { cs_svr_ptr->Start(); },
                                 [cs_svr_ptr] { cs_svr_ptr->Stop(); });

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
    cs_cli_ptr->SendMsg(msg_buf_ptr);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  svr_sys_ptr->Stop();
  t_svr.join();

  cli_sys_ptr->Stop();
  t_cli.join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());
}

class TestMsg {
  T_CLASS_SERIALIZE(&code &data)
 public:
  uint32_t code;
  std::string data;
};

TEST(BOOST_ASIO_TEST, CS_TestMsg) {
  AsioDebugTool::Ins().Reset();

  auto cli_sys_ptr = std::make_shared<AsioExecutor>(2);
  auto svr1_sys_ptr = std::make_shared<AsioExecutor>(2);
  auto svr2_sys_ptr = std::make_shared<AsioExecutor>(2);

  AsioCsClient::Cfg cs_cli_cfg;
  cs_cli_cfg.svr_ep = asio::ip::tcp::endpoint{asio::ip::address_v4({127, 0, 0, 1}), 57634};
  auto cs_cli_ptr = std::make_shared<AsioCsClient>(cli_sys_ptr->IO(), cs_cli_cfg);
  cs_cli_ptr->RegisterMsgHandleFunc(
      [cs_cli_ptr](std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
        TestMsg msg;
        boost::archive::binary_iarchive iar(*msg_buf_ptr);
        iar >> msg;
        DBG_PRINT("cli get a msg, code: %d, data: %s", msg.code, msg.data.c_str());
      });

  cli_sys_ptr->RegisterSvrFunc(std::function<void()>(), [cs_cli_ptr] { cs_cli_ptr->Stop(); });

  std::thread t_cli([cli_sys_ptr] {
    DBG_PRINT("cli_sys_ptr start");
    cli_sys_ptr->Start();
    cli_sys_ptr->Join();
    DBG_PRINT("cli_sys_ptr exit");
  });

  // svr1
  std::thread t_svr1([svr1_sys_ptr] {
    DBG_PRINT("svr1_sys_ptr start");
    auto cs_svr_ptr = std::make_shared<AsioCsServer>(svr1_sys_ptr->IO(), AsioCsServer::Cfg());
    cs_svr_ptr->RegisterMsgHandleFunc(
        [cs_svr_ptr](boost::asio::ip::tcp::endpoint ep, std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
          TestMsg msg;
          boost::archive::binary_iarchive iar(*msg_buf_ptr);
          iar >> msg;
          DBG_PRINT("svr1 get a msg from %s, code: %d, data: %s", TcpEp2Str(ep).c_str(), msg.code, msg.data.c_str());

          msg.data = "svr1 echo " + msg.data;
          std::shared_ptr<boost::asio::streambuf> send_msg_buf_ptr = std::make_shared<boost::asio::streambuf>();
          boost::archive::binary_oarchive oar(*send_msg_buf_ptr);
          oar << msg;
          cs_svr_ptr->SendMsg(ep, send_msg_buf_ptr);
        });

    svr1_sys_ptr->RegisterSvrFunc([cs_svr_ptr] { cs_svr_ptr->Start(); },
                                  [cs_svr_ptr] { cs_svr_ptr->Stop(); });

    svr1_sys_ptr->Start();
    svr1_sys_ptr->Join();
    DBG_PRINT("svr1_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    TestMsg msg{111, "test msg 111"};
    std::shared_ptr<boost::asio::streambuf> msg_buf_ptr = std::make_shared<boost::asio::streambuf>();
    boost::archive::binary_oarchive oar(*msg_buf_ptr);
    oar << msg;
    cs_cli_ptr->SendMsg(msg_buf_ptr);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  svr1_sys_ptr->Stop();
  t_svr1.join();

  // svr2
  std::thread t_svr2([svr2_sys_ptr] {
    DBG_PRINT("svr2_sys_ptr start");
    auto cs_svr_ptr = std::make_shared<AsioCsServer>(svr2_sys_ptr->IO(), AsioCsServer::Cfg());
    cs_svr_ptr->RegisterMsgHandleFunc(
        [cs_svr_ptr](boost::asio::ip::tcp::endpoint ep, std::shared_ptr<boost::asio::streambuf> msg_buf_ptr) {
          TestMsg msg;
          boost::archive::binary_iarchive iar(*msg_buf_ptr);
          iar >> msg;
          DBG_PRINT("svr2 get a msg from %s, code: %d, data: %s", TcpEp2Str(ep).c_str(), msg.code, msg.data.c_str());

          msg.data = "svr2 echo " + msg.data;
          std::shared_ptr<boost::asio::streambuf> send_msg_buf_ptr = std::make_shared<boost::asio::streambuf>();
          boost::archive::binary_oarchive oar(*send_msg_buf_ptr);
          oar << msg;
          cs_svr_ptr->SendMsg(ep, send_msg_buf_ptr);
        });

    svr2_sys_ptr->RegisterSvrFunc([cs_svr_ptr] { cs_svr_ptr->Start(); },
                                  [cs_svr_ptr] { cs_svr_ptr->Stop(); });

    svr2_sys_ptr->Start();
    svr2_sys_ptr->Join();
    DBG_PRINT("svr2_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  {
    TestMsg msg{222, "test msg 222"};
    std::shared_ptr<boost::asio::streambuf> msg_buf_ptr = std::make_shared<boost::asio::streambuf>();
    boost::archive::binary_oarchive oar(*msg_buf_ptr);
    oar << msg;
    cs_cli_ptr->SendMsg(msg_buf_ptr);
  }

  {
    TestMsg msg{333, "test msg 333"};
    std::shared_ptr<boost::asio::streambuf> msg_buf_ptr = std::make_shared<boost::asio::streambuf>();
    boost::archive::binary_oarchive oar(*msg_buf_ptr);
    oar << msg;
    cs_cli_ptr->SendMsg(msg_buf_ptr);
  }

  {
    TestMsg msg{444, "test msg 444"};
    std::shared_ptr<boost::asio::streambuf> msg_buf_ptr = std::make_shared<boost::asio::streambuf>();
    boost::archive::binary_oarchive oar(*msg_buf_ptr);
    oar << msg;
    cs_cli_ptr->SendMsg(msg_buf_ptr);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  svr2_sys_ptr->Stop();
  t_svr2.join();

  cli_sys_ptr->Stop();
  t_cli.join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());
}

}  // namespace ytlib
