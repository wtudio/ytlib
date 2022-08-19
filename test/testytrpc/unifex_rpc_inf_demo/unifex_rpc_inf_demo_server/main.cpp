#include <string>
#include <tuple>

#include "ytlib/misc/misc_macro.h"

#include "ytlib/ytrpc/unifex_rpc_inf/unifex_rpc_server_inf.hpp"

#include "Demo.pb.h"
#include "Demo.unifex_rpc_inf_for_asio_rpc.pb.h"

using namespace std;
using namespace ytlib;
using namespace ytlib::ytrpc;

std::shared_ptr<UnifexRpcClient> unifex_rpc_cli_ptr;

class DemoServiceImpl : public demo::DemoService {
 public:
  virtual auto Login(const shared_ptr<const UnifexRpcContextInf>& ctx_ptr, const demo::LoginReq& req)
      -> unifex::task<tuple<UnifexRpcStatusInf, demo::LoginRsp>> override {
    auto unifex_rpc_proxy = GenCommRpcProxy(unifex_rpc_cli_ptr);

    // 定制ctx
    shared_ptr<UnifexRpcContext> native_ctx_ptr = ctx_ptr->GetNativeCtxHandle(KUnifexRpcName);
    native_ctx_ptr->ContextKv.emplace("kkk", "vvv");

    const auto& [status, rsp] = co_await unifex_rpc_proxy->Login(ctx_ptr, req);

    // 定制status
    AsioRpcClient::Status native_status = *(status.GetNativeStatusHandle(KAsioRpcName));

    co_return {status, move(rsp)};
  }

  virtual auto Logout(const shared_ptr<const UnifexRpcContextInf>& ctx_ptr, const demo::LogoutReq& req)
      -> unifex::task<tuple<UnifexRpcStatusInf, demo::LogoutRsp>> override {
    demo::LogoutRsp rsp;
    rsp.set_code(0);
    rsp.set_msg("echo " + req.msg());
    co_return {UnifexRpcStatusInf(UnifexRpcStatusInf::Code::OK), move(rsp)};
  }
};

int32_t main(int32_t argc, char** argv) {
  auto asio_sys_ptr = std::make_shared<AsioExecutor>(4);
  asio_sys_ptr->EnableStopSignal();

  // 初始化client
  UnifexRpcClient::Cfg cfg;
  cfg.svr_ep = ip::tcp::endpoint{ip::address_v4({127, 0, 0, 1}), 55399};
  unifex_rpc_cli_ptr = make_shared<UnifexRpcClient>(asio_sys_ptr->IO(), cfg);

  // 初始化server
  AsioRpcServer::Cfg cfg;
  auto svr_ptr = std::make_shared<AsioRpcServer>(asio_sys_ptr->IO(), cfg);

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

  // 注册service
  auto asio_rpc_service = GenAsioRpcService(std::make_shared<DemoServiceImpl>(), comm_ctx_ptr);
  svr_ptr->RegisterService(asio_rpc_service);

  // 启动server
  asio_sys_ptr->RegisterSvrFunc([svr_ptr] { svr_ptr->Start(); },
                                [svr_ptr] { svr_ptr->Stop(); });

  asio_sys_ptr->RegisterSvrFunc(std::function<void()>(),
                                [unifex_rpc_cli_ptr] { unifex_rpc_cli_ptr->Stop(); });

  asio_sys_ptr->Start();
  asio_sys_ptr->Join();

  return 0;
}
