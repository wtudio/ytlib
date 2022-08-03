#include <string>
#include <tuple>

#include "ytlib/misc/misc_macro.h"

#include "ytlib/ytrpc/unifex_rpc/unifex_rpc_server.hpp"

#include "Demo.pb.h"
#include "Demo.unifex_rpc.pb.h"

using namespace std;
using namespace ytlib;

class DemoServiceImpl : public demo::DemoService {
 public:
  virtual unifex::task<std::tuple<ytrpc::UnifexRpcStatus, demo::LoginRsp>> Login(const std::shared_ptr<const ytrpc::UnifexRpcContext>& ctx_ptr, const demo::LoginReq& req) override {
    demo::LoginRsp rsp;
    rsp.set_code(0);
    rsp.set_msg("echo " + req.msg());
    co_return {ytrpc::UnifexRpcStatus(ytrpc::UnifexRpcStatus::Code::OK), std::move(rsp)};
  }

  virtual unifex::task<std::tuple<ytrpc::UnifexRpcStatus, demo::LogoutRsp>> Logout(const std::shared_ptr<const ytrpc::UnifexRpcContext>& ctx_ptr, const demo::LogoutReq& req) override {
    demo::LogoutRsp rsp;
    rsp.set_code(0);
    rsp.set_msg("echo " + req.msg());
    co_return {ytrpc::UnifexRpcStatus(ytrpc::UnifexRpcStatus::Code::OK), std::move(rsp)};
  }
};

int32_t main(int32_t argc, char** argv) {
  ytrpc::UnifexRpcServer::Cfg cfg;
  auto svr_ptr = std::make_shared<ytrpc::UnifexRpcServer>(cfg);

  svr_ptr->RegisterService(std::make_shared<DemoServiceImpl>());

  return 0;
}
