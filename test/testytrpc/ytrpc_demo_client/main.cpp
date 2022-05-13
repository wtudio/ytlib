#include <string>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"

#include "ytrpc_client.hpp"

#include "Demo.pb.h"
#include "Demo.ytrpc.pb.h"

#include <google/protobuf/util/json_util.h>

using namespace std;
using namespace ytlib;

// 将pb包转换为格式化json字符串
std::string Pb2PrettyJson(const google::protobuf::Message& st) {
  google::protobuf::util::JsonPrintOptions op;
  op.always_print_primitive_fields = true;
  op.always_print_enums_as_ints = false;
  op.preserve_proto_field_names = true;
  op.add_whitespace = true;
  std::string str;
  google::protobuf::util::MessageToJsonString(st, &str, op);
  return str;
}

// 将pb包转换为紧凑型json字符串
std::string Pb2CompactJson(const google::protobuf::Message& st) {
  google::protobuf::util::JsonPrintOptions op;
  op.always_print_primitive_fields = true;
  op.always_print_enums_as_ints = false;
  op.preserve_proto_field_names = true;
  op.add_whitespace = false;
  std::string str;
  google::protobuf::util::MessageToJsonString(st, &str, op);
  return str;
}

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  AsioDebugTool::Ins().Reset();

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(4);

  ytrpc::RpcClient::Cfg cfg;
  cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 55399};
  auto cli_ptr = std::make_shared<ytrpc::RpcClient>(asio_sys_ptr->IO(), cfg);

  asio_sys_ptr->RegisterSvrFunc(std::function<void()>(),
                                [cli_ptr] { cli_ptr->Stop(); });

  boost::asio::co_spawn(
      *(asio_sys_ptr->IO()),
      [cli_ptr]() -> boost::asio::awaitable<void> {
        uint32_t ct = 10;
        auto demo_service_proxy_ptr = std::make_shared<demo::DemoServiceProxy>(cli_ptr);
        while (ct--) {
          auto ctx = std::make_shared<ytrpc::Context>();
          ctx->SetTimeout(std::chrono::milliseconds(3000));

          demo::LoginReq req;
          req.set_msg("test msg " + std::to_string(ct));

          DBG_PRINT("ctx: %s", ctx->ToString().c_str());
          DBG_PRINT("req: %s", Pb2PrettyJson(req).c_str());

          demo::LoginRsp rsp;
          auto status = co_await demo_service_proxy_ptr->Login(ctx, req, rsp);

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
