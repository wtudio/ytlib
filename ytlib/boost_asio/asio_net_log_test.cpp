#include <gtest/gtest.h>

#include "asio_net_log_cli.hpp"
#include "asio_net_log_svr.hpp"
#include "asio_tools.hpp"
#include "boost_log.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

using namespace boost::asio;
using namespace std;

TEST(BOOST_ASIO_TEST, NET_LOG) {
  AsioDebugTool::Ins().Reset();

  auto cli_sys_ptr = std::make_shared<AsioExecutor>(1);
  auto svr1_sys_ptr = std::make_shared<AsioExecutor>(2);
  auto svr2_sys_ptr = std::make_shared<AsioExecutor>(2);

  // cli
  AsioNetLogClient::Cfg net_log_client_cfg;
  net_log_client_cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 52684};
  net_log_client_cfg.timer_dt = std::chrono::milliseconds(100);

  auto net_log_cli_ptr = std::make_shared<AsioNetLogClient>(cli_sys_ptr->IO(), net_log_client_cfg);
  cli_sys_ptr->RegisterSvrFunc(std::function<void()>(),
                               [net_log_cli_ptr] { net_log_cli_ptr->Stop(); });

  YTBLCtr::Ins().EnableConsoleLog();
  YTBLCtr::Ins().EnableFileLog("./test");
  YTBLCtr::Ins().EnableNetLog(net_log_cli_ptr);

  thread t_cli([cli_sys_ptr] {
    cli_sys_ptr->Start();
    cli_sys_ptr->Join();
    DBG_PRINT("cli_sys_ptr exit");
  });

  // svr1
  thread t_svr1([svr1_sys_ptr] {
    AsioNetLogServer::Cfg cfg;
    cfg.timer_dt = std::chrono::milliseconds(100);
    auto lgsvr_ptr = std::make_shared<AsioNetLogServer>(svr1_sys_ptr->IO(), cfg);
    svr1_sys_ptr->RegisterSvrFunc([lgsvr_ptr] { lgsvr_ptr->Start(); },
                                  [lgsvr_ptr] { lgsvr_ptr->Stop(); });

    svr1_sys_ptr->Start();
    svr1_sys_ptr->Join();
    DBG_PRINT("svr1_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  YTBL_SET_LEVEL(info);

  // cli建立连接并发送到svr1上
  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  svr1_sys_ptr->Stop();
  t_svr1.join();

  // svr2
  thread t_svr2([svr2_sys_ptr] {
    AsioNetLogServer::Cfg cfg;
    cfg.port = 52684;
    cfg.log_path = "./log2";
    cfg.timer_dt = std::chrono::milliseconds(100);
    cfg.max_no_data_duration = std::chrono::seconds(10);
    auto lgsvr_ptr = std::make_shared<AsioNetLogServer>(svr2_sys_ptr->IO(), cfg);
    svr2_sys_ptr->RegisterSvrFunc([lgsvr_ptr] { lgsvr_ptr->Start(); },
                                  [lgsvr_ptr] { lgsvr_ptr->Stop(); });
    svr2_sys_ptr->Start();
    svr2_sys_ptr->Join();
    DBG_PRINT("svr2_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // 连接已经被svr1关闭，cli虽然能正常发送，但会触发svr端回复rst
  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // cli 再次发送，触发cli端连接异常，关闭cli端连接
  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // cli建立连接并发送到svr2
  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // cli发送到svr2
  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  svr2_sys_ptr->Stop();
  t_svr2.join();

  cli_sys_ptr->Stop();
  t_cli.join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());
}

}  // namespace ytlib
