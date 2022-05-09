#include <string>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"

#include "ytrpc_server.hpp"

#include "Demo.pb.h"
#include "Demo.ytrpc.pb.h"

using namespace std;
using namespace ytlib;

class DemoService : public demo::DemoService {
 public:
  virtual boost::asio::awaitable<ytrpc::Status> Login(std::shared_ptr<ytrpc::Context> ctx, const demo::LoginReq& req, demo::LoginRsp& rsp) override {
    rsp.set_code(0);
    rsp.set_msg("echo " + req.msg());
    co_return ytrpc::Status(ytrpc::StatusCode::OK);
  }

  virtual boost::asio::awaitable<ytrpc::Status> Logout(std::shared_ptr<ytrpc::Context> ctx, const demo::LogoutReq& req, demo::LogoutRsp& rsp) override {
    rsp.set_code(0);
    rsp.set_msg("echo " + req.msg());
    co_return ytrpc::Status(ytrpc::StatusCode::OK);
  }
};

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  AsioDebugTool::Ins().Reset();

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(4);

  ytrpc::RpcServer::Cfg cfg;
  cfg.port = 55399;
  auto svr_ptr = std::make_shared<ytrpc::RpcServer>(asio_sys_ptr->IO(), cfg);
  auto demo_service = std::make_shared<DemoService>();
  svr_ptr->RegisterService(demo_service);

  asio_sys_ptr->RegisterSvrFunc([svr_ptr] { svr_ptr->Start(); },
                                [svr_ptr] { svr_ptr->Stop(); });

  asio_sys_ptr->Start();
  asio_sys_ptr->Join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  google::protobuf::ShutdownProtobufLibrary();

  DBG_PRINT("********************end test*******************");
  return 0;
}
