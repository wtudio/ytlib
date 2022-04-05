#include <gtest/gtest.h>

#include "asio_tools.hpp"
#include "boost_log.hpp"
#include "net_log.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

using namespace boost::asio;
using namespace std;

TEST(BOOST_ASIO_TEST, LOG) {
  AsioDebugTool::Ins().Reset();

  auto cli_sys_ptr = std::make_shared<AsioExecutor>(1);
  auto svr1_sys_ptr = std::make_shared<AsioExecutor>(2);
  auto svr2_sys_ptr = std::make_shared<AsioExecutor>(2);

  // cli
  auto net_log_cli_ptr = std::make_shared<NetLogClient>(cli_sys_ptr->IO(), boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 50001});
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
    auto lgsvr_ptr = std::make_shared<LogSvr>(svr1_sys_ptr->IO(), LogSvrCfg());
    svr1_sys_ptr->RegisterSvrFunc([lgsvr_ptr] { lgsvr_ptr->Start(); },
                                  [lgsvr_ptr] { lgsvr_ptr->Stop(); });

    svr1_sys_ptr->Start();
    svr1_sys_ptr->Join();
    DBG_PRINT("svr1_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  YTBL_SET_LEVEL(info);

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
    LogSvrCfg cfg;
    cfg.port = 50001;
    cfg.log_path = "./log2";
    cfg.timer_dt = std::chrono::seconds(1);
    cfg.max_no_data_duration = std::chrono::seconds(10);
    auto lgsvr_ptr = std::make_shared<LogSvr>(svr2_sys_ptr->IO(), cfg);
    svr2_sys_ptr->RegisterSvrFunc([lgsvr_ptr] { lgsvr_ptr->Start(); },
                                  [lgsvr_ptr] { lgsvr_ptr->Stop(); });
    svr2_sys_ptr->Start();
    svr2_sys_ptr->Join();
    DBG_PRINT("svr2_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(5));

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
