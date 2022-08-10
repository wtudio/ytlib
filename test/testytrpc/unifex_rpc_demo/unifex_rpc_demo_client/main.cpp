#include <string>
#include <tuple>

#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>

#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/pb_tools/pb_tools.hpp"

#include "ytlib/ytrpc/unifex_rpc/unifex_rpc_client.hpp"

#include "Demo.pb.h"
#include "Demo.unifex_rpc.pb.h"

using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
  auto asio_sys_ptr = std::make_shared<AsioExecutor>(4);
  asio_sys_ptr->Start();

  ytrpc::UnifexRpcClient::Cfg cfg;
  cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 51965};
  auto cli_ptr = std::make_shared<ytrpc::UnifexRpcClient>(asio_sys_ptr->IO(), cfg);

  auto co_work = [cli_ptr]() -> unifex::task<void> {
    uint32_t ct = 10;
    auto demo_service_proxy_ptr = std::make_shared<demo::DemoServiceProxy>(cli_ptr);
    while (ct--) {
      auto ctx_ptr = std::make_shared<ytrpc::UnifexRpcContext>();
      ctx_ptr->SetTimeout(std::chrono::milliseconds(3000));

      demo::LoginReq req;
      req.set_msg("test msg " + std::to_string(ct));

      DBG_PRINT("ctx: %s", ctx_ptr->ToString().c_str());
      DBG_PRINT("req: %s", Pb2PrettyJson(req).c_str());

      const auto& [status, rsp] = co_await demo_service_proxy_ptr->Login(ctx_ptr, req);

      DBG_PRINT("status: %s", status.ToString().c_str());
      DBG_PRINT("rsp: %s", Pb2PrettyJson(rsp).c_str());
    }
    co_return;
  };

  unifex::sync_wait(co_work());

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();

  return 0;
}
