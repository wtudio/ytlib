#include <string>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/pb_tools/pb_tools.hpp"

#include "ytlib/ytrpc/asio_rpc/asio_rpc_client.hpp"

#include "Demo.asio_rpc.pb.h"
#include "Demo.pb.h"

using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  AsioDebugTool::Ins().Reset();

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(4);
  asio_sys_ptr->EnableStopSignal();

  ytrpc::AsioRpcClient::Cfg cfg;
  cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 55399};
  auto cli_ptr = std::make_shared<ytrpc::AsioRpcClient>(asio_sys_ptr->IO(), cfg);

  asio_sys_ptr->RegisterSvrFunc(std::function<void()>(),
                                [cli_ptr] { cli_ptr->Stop(); });

  boost::asio::co_spawn(
      *(asio_sys_ptr->IO()),
      [cli_ptr]() -> boost::asio::awaitable<void> {
        uint32_t ct = 10;
        auto demo_service_proxy_ptr = std::make_shared<demo::DemoServiceProxy>(cli_ptr);
        while (ct--) {
          auto ctx_ptr = std::make_shared<ytrpc::Context>();
          ctx_ptr->SetTimeout(std::chrono::milliseconds(3000));

          demo::LoginReq req;
          req.set_msg("test msg " + std::to_string(ct));

          DBG_PRINT("ctx: %s", ctx_ptr->ToString().c_str());
          DBG_PRINT("req: %s", Pb2PrettyJson(req).c_str());

          demo::LoginRsp rsp;
          auto status = co_await demo_service_proxy_ptr->Login(ctx_ptr, req, rsp);

          DBG_PRINT("status: %s", status.ToString().c_str());
          DBG_PRINT("rsp: %s", Pb2PrettyJson(rsp).c_str());
        }
        co_return;
      },
      boost::asio::detached);

  asio_sys_ptr->Start();
  asio_sys_ptr->Join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  google::protobuf::ShutdownProtobufLibrary();

  DBG_PRINT("********************end test*******************");
  return 0;
}
