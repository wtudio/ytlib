#include <thread>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/asio_net_log_svr.hpp"
#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  AsioDebugTool::Ins().Reset();

  AsioNetLogServer::Cfg cfg;
  cfg.port = 50009;
  cfg.log_path = "./log";
  cfg.mgr_timer_dt = std::chrono::seconds(60);
  cfg.timer_dt = std::chrono::seconds(10);
  cfg.max_no_data_duration = std::chrono::seconds(120);

  auto log_svr_sys_ptr = std::make_shared<AsioExecutor>(8);
  auto net_log_svr_ptr = std::make_shared<AsioNetLogServer>(log_svr_sys_ptr->IO(), cfg);
  log_svr_sys_ptr->RegisterSvrFunc([net_log_svr_ptr] { net_log_svr_ptr->Start(); },
                                   [net_log_svr_ptr] { net_log_svr_ptr->Stop(); });

  log_svr_sys_ptr->Start();
  log_svr_sys_ptr->Join();
  DBG_PRINT("log_svr_sys_ptr exit");

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  DBG_PRINT("********************end test*******************");
  return 0;
}
