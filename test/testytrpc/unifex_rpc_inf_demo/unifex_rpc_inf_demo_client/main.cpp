#include <string>
#include <tuple>

#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>

#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/pb_tools/pb_tools.hpp"

#include "ytlib/ytrpc/unifex_rpc_inf/unifex_rpc_client_inf_filter.hpp"
#include "ytlib/ytrpc/unifex_rpc_inf/unifex_rpc_context_inf.hpp"
#include "ytlib/ytrpc/unifex_rpc_inf/unifex_rpc_status_inf.hpp"

#include "Demo.pb.h"
#include "Demo.unifex_rpc_inf_for_asio_rpc.pb.h"

using namespace std;
using namespace ytlib;
using namespace ytlib::ytrpc;
using namespace boost::asio;

int32_t main(int32_t argc, char** argv) {
  // 初始化两种RPC的client句柄
  auto asio_sys_ptr = make_shared<AsioExecutor>(4);
  asio_sys_ptr->EnableStopSignal();
  asio_sys_ptr->Start();

  AsioRpcClient::Cfg cfg;
  cfg.svr_ep = ip::tcp::endpoint{ip::address_v4({127, 0, 0, 1}), 55399};
  auto asio_rpc_cli_ptr = make_shared<AsioRpcClient>(asio_sys_ptr->IO(), cfg);

  UnifexRpcClient::Cfg cfg;
  cfg.svr_ep = ip::tcp::endpoint{ip::address_v4({127, 0, 0, 1}), 55399};
  auto unifex_rpc_cli_ptr = make_shared<UnifexRpcClient>(asio_sys_ptr->IO(), cfg);

  // ------------------------------------------------

  // 过滤器，打印日志
  RpcFilterHandle debug_log = [](const Ctx& ctx, const google::protobuf::Message& req, const RpcHandle& next) -> task<tuple<Status, Rsp>> {
    uint64_t begin_time = GetCurTimestampMs();
    auto [status, rsp] = next(ctx, req);
    uint64_t end_time = GetCurTimestampMs();

    printf("rpc call time: %llums, req: %s, rsp: %s",
           end_time - begin_time, Pb2PrettyJson(req).c_str(), Pb2PrettyJson(rsp).c_str());
    co_return std::tie(status, rsp);
  };

  // 设置通用context
  shared_ptr<UnifexRpcContextInf> comm_ctx_ptr = make_shared<UnifexRpcContextInf>();
  comm_ctx_ptr->SetTimeout(chrono::milliseconds(5000));
  comm_ctx_ptr.RegFilter(debug_log);

  // 启动一个协程
  auto co_work = [&]() -> unifex::task<void> {
    // 创建两种RPC的proxy
    auto asio_rpc_proxy = GenCommRpcProxy(asio_rpc_cli_ptr, comm_ctx_ptr);
    auto unifex_rpc_proxy = GenCommRpcProxy(unifex_rpc_cli_ptr);

    // 创建通用context
    shared_ptr<UnifexRpcContextInf> ctx_ptr = make_shared<UnifexRpcContextInf>();
    ctx_ptr->SetTimeout(chrono::milliseconds(3000));

    // 发起第一种RPC调用
    {
      demo::LoginReq req;
      req.set_msg("test msg " + to_string(ct));

      const auto& [status, rsp] = co_await asio_rpc_proxy->Login(ctx_ptr, req);

      AsioRpcClient::Status native_status = *(status.GetNativeStatusHandle(KAsioRpcName));
    }

    // 复用context发起第二种RPC调用
    {
      demo::LoginReq req;
      req.set_msg("test msg " + to_string(ct));

      shared_ptr<UnifexRpcContext> native_ctx_ptr = ctx_ptr->GetNativeCtxHandle(KUnifexRpcName);
      native_ctx_ptr->ContextKv.emplace("kkk", "vvv");

      const auto& [status, rsp] = co_await unifex_rpc_proxy->Login(ctx_ptr, req);

      UnifexRpcClient::Status native_status = *(status.GetNativeStatusHandle(KUnifexRpcName));
    }

    co_return;
  };

  unifex::sync_wait(co_work());

  // ------------------------------------------------

  asio_rpc_cli_ptr->Stop();
  unifex_rpc_cli_ptr->Stop();

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();

  return 0;
}
