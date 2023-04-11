#include <string>

#include "ytlib/boost_tools_asio/asio_debug_tools.hpp"
#include "ytlib/boost_tools_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"

#include "ytlib/ytrpc/asio_rpc/asio_rpc_server.hpp"

#include "helloworld.asio_rpc.pb.h"
#include "helloworld.pb.h"

using namespace std;
using namespace ytlib;

class GreeterImpl : public trpc::test::helloworld::Greeter {
 public:
  virtual boost::asio::awaitable<ytrpc::AsioRpcStatus> SayHello(const std::shared_ptr<const ytrpc::AsioRpcContext>& ctx_ptr, const trpc::test::helloworld::HelloRequest& req, trpc::test::helloworld::HelloReply& rsp) override {
    rsp.set_msg("Hello, " + req.msg());
    co_return ytrpc::AsioRpcStatus(ytrpc::AsioRpcStatus::Code::OK);
  }
};

int32_t main(int32_t argc, char** argv) {
  AsioDebugTool::Ins().Reset();

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(8);
  asio_sys_ptr->EnableStopSignal();

  ytrpc::AsioRpcServer::Cfg cfg;
  auto svr_ptr = std::make_shared<ytrpc::AsioRpcServer>(asio_sys_ptr->IO(), cfg);

  svr_ptr->RegisterService(std::make_shared<GreeterImpl>());

  asio_sys_ptr->RegisterSvrFunc([svr_ptr] { svr_ptr->Start(); },
                                [svr_ptr] { svr_ptr->Stop(); });

  asio_sys_ptr->Start();
  asio_sys_ptr->Join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  return 0;
}
