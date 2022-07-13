#include <string>

#include <boost/asio/experimental/promise.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

#include "ytlib/ytrpc/asio_rpc/asio_rpc_client.hpp"

#include "helloworld.asio_rpc.pb.h"
#include "helloworld.pb.h"

using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
  AsioDebugTool::Ins().Reset();

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(8);
  asio_sys_ptr->EnableStopSignal();

  ytrpc::AsioRpcClient::Cfg cfg;
  cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 55399};
  auto cli_ptr = std::make_shared<ytrpc::AsioRpcClient>(asio_sys_ptr->IO(), cfg);

  asio_sys_ptr->RegisterSvrFunc(std::function<void()>(),
                                [cli_ptr] { cli_ptr->Stop(); });

  asio_sys_ptr->Start();

  auto co_future = boost::asio::co_spawn(
      *(asio_sys_ptr->IO()),
      [cli_ptr, asio_sys_ptr]() -> boost::asio::awaitable<void> {
        auto proxy_ptr = std::make_shared<trpc::test::helloworld::GreeterProxy>(cli_ptr);

        const uint32_t concurrency_num = 1000;
        const uint32_t try_num = 100;

        std::atomic_int successed_num = 0;
        std::atomic_uint64_t total_time = 0;

        std::vector<std::string> request_msgs(concurrency_num);
        for (uint32_t ii = 0; ii < concurrency_num; ++ii) {
          request_msgs[ii] = std::to_string(concurrency_num + ii);
        }

        uint64_t all_begin_time = GetCurTimestampMs();
        for (uint32_t ii = 0; ii < try_num; ++ii) {
          std::list<boost::asio::experimental::promise<void(std::exception_ptr), boost::asio::any_io_executor>> promise_list;
          for (auto& request_msg : request_msgs) {
            promise_list.emplace_back(
                boost::asio::co_spawn(
                    *(asio_sys_ptr->IO()),
                    [&proxy_ptr, &request_msg, &successed_num, &total_time]() -> boost::asio::awaitable<void> {
                      trpc::test::helloworld::HelloRequest req;
                      trpc::test::helloworld::HelloReply rsp;
                      req.set_msg(request_msg);

                      auto ctx_ptr = std::make_shared<ytrpc::Context>();
                      ctx_ptr->SetTimeout(std::chrono::milliseconds(1000));

                      uint64_t begin_time = GetCurTimestampMs();
                      auto status = co_await proxy_ptr->SayHello(ctx_ptr, req, rsp);
                      uint64_t end_time = GetCurTimestampMs();

                      total_time += (end_time - begin_time);

                      if (status && rsp.msg() == ("Hello, " + request_msg)) {
                        ++successed_num;
                      } else {
                        printf("task request error: %s\n", status.ToString().c_str());
                      }

                      co_return;
                    },
                    boost::asio::experimental::use_promise));
          }

          auto all_promise = boost::asio::experimental::promise<>::all(std::move(promise_list));
          co_await all_promise.async_wait(boost::asio::use_awaitable);
        }
        uint64_t all_end_time = GetCurTimestampMs();

        printf("all done... succ: %d, timecost(ms): %llu, average timecost(ms): %llu\n", static_cast<int>(successed_num), all_end_time - all_begin_time, static_cast<uint64_t>(total_time) / try_num / concurrency_num);

        co_return;
      },
      boost::asio::use_future);

  co_future.wait();

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  return 0;
}
