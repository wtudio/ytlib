#include <string>
#include <tuple>

#include <unifex/inline_scheduler.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/when_all.hpp>

#include "ytlib/boost_tools_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

#include "ytlib/ytrpc/unifex_rpc/unifex_rpc_client.hpp"

#include "helloworld.pb.h"
#include "helloworld.unifex_rpc.pb.h"

using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
  auto asio_sys_ptr = std::make_shared<AsioExecutor>(8);
  asio_sys_ptr->Start();

  ytrpc::UnifexRpcClient::Cfg cfg;
  cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 55399};
  auto cli_ptr = std::make_shared<ytrpc::UnifexRpcClient>(asio_sys_ptr->IO(), cfg);

  auto co_work = [cli_ptr]() -> unifex::task<void> {
    unifex::async_scope scope;

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
      AsyncLatch latch(concurrency_num);

      for (auto& request_msg : request_msgs) {
        scope.spawn(unifex::co_invoke([&latch, &proxy_ptr, &request_msg, &successed_num, &total_time]() -> unifex::task<void> {
          trpc::test::helloworld::HelloRequest req;
          req.set_msg(request_msg);

          auto ctx_ptr = std::make_shared<ytrpc::UnifexRpcContext>();
          ctx_ptr->SetTimeout(std::chrono::milliseconds(1000));

          uint64_t begin_time = GetCurTimestampMs();
          const auto& [status, rsp] = co_await proxy_ptr->SayHello(ctx_ptr, req);
          uint64_t end_time = GetCurTimestampMs();

          total_time += (end_time - begin_time);

          if (status && rsp.msg() == ("Hello, " + request_msg)) {
            ++successed_num;
          } else {
            printf("task request error: %s\n", status.ToString().c_str());
          }
          latch.CountDown();

          co_return;
        }));
      }

      co_await unifex::on(unifex::inline_scheduler{}, latch.AsyncWait());
    }
    uint64_t all_end_time = GetCurTimestampMs();

    printf("all done... succ: %d, timecost(ms): %llu, average concurrency timecost(ms): %llu, average rpc timecost(ms): %llu\n",
           static_cast<int>(successed_num),
           all_end_time - all_begin_time,
           (all_end_time - all_begin_time) / try_num,
           static_cast<uint64_t>(total_time) / try_num / concurrency_num);

    co_await unifex::on(unifex::inline_scheduler{}, scope.cleanup());

    co_return;
  };

  unifex::sync_wait(co_work());

  cli_ptr->Stop();

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();

  return 0;
}
