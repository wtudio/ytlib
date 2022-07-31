#include <string>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"

#include "ytlib/ytrpc/asio_rpc/asio_rpc_server.hpp"

#include "Demo.asio_rpc.pb.h"
#include "Demo.pb.h"

using namespace std;
using namespace ytlib;

class DemoServiceImpl : public demo::DemoService {
 public:
  virtual boost::asio::awaitable<ytrpc::AsioRpcStatus> Login(const std::shared_ptr<const ytrpc::AsioRpcContext>& ctx_ptr, const demo::LoginReq& req, demo::LoginRsp& rsp) override {
    rsp.set_code(0);
    rsp.set_msg("echo " + req.msg());
    co_return ytrpc::AsioRpcStatus(ytrpc::AsioRpcStatus::Code::OK);
  }

  virtual boost::asio::awaitable<ytrpc::AsioRpcStatus> Logout(const std::shared_ptr<const ytrpc::AsioRpcContext>& ctx_ptr, const demo::LogoutReq& req, demo::LogoutRsp& rsp) override {
    rsp.set_code(0);
    rsp.set_msg("echo " + req.msg());
    co_return ytrpc::AsioRpcStatus(ytrpc::AsioRpcStatus::Code::OK);
  }
};

int32_t main(int32_t argc, char** argv) {
  AsioDebugTool::Ins().Reset();

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(4);
  asio_sys_ptr->EnableStopSignal();

  ytrpc::AsioRpcServer::Cfg cfg;
  auto svr_ptr = std::make_shared<ytrpc::AsioRpcServer>(asio_sys_ptr->IO(), cfg);

  svr_ptr->RegisterService(std::make_shared<DemoServiceImpl>());

  asio_sys_ptr->RegisterSvrFunc([svr_ptr] { svr_ptr->Start(); },
                                [svr_ptr] { svr_ptr->Stop(); });

  asio_sys_ptr->Start();
  asio_sys_ptr->Join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  return 0;
}
