#include <string>
#include <tuple>

#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/when_all.hpp>

#include "ytlib/boost_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"

#include "ytlib/ytrpc/unifex_rpc/unifex_rpc_client.hpp"

#include "helloworld.pb.h"
#include "helloworld.unifex_rpc.pb.h"

using namespace std;
using namespace ytlib;

template <typename Rng, std::size_t... I>
constexpr auto when_all_vec_impl(const Rng& rng, std::index_sequence<I...>) {
  return unifex::when_all(std::move(*rng[I])...);
}

template <typename Rng, std::size_t N, typename Indices = std::make_index_sequence<N>>
constexpr auto when_all_vec(const Rng& rng) {
  return when_all_vec_impl(rng, Indices{});
}

int32_t main(int32_t argc, char** argv) {
  auto asio_sys_ptr = std::make_shared<AsioExecutor>(4);
  asio_sys_ptr->Start();

  ytrpc::UnifexRpcClient::Cfg cfg;
  // cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 51965};
  cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 55399};
  auto cli_ptr = std::make_shared<ytrpc::UnifexRpcClient>(asio_sys_ptr->IO(), cfg);

  auto co_work = [cli_ptr]() -> unifex::task<void> {
    auto proxy_ptr = std::make_shared<trpc::test::helloworld::GreeterProxy>(cli_ptr);

    constexpr uint32_t concurrency_num = 2;
    constexpr uint32_t try_num = 1;

    std::atomic_int successed_num = 0;
    std::atomic_uint64_t total_time = 0;

    std::vector<std::string> request_msgs(concurrency_num);
    for (uint32_t ii = 0; ii < concurrency_num; ++ii) {
      request_msgs[ii] = std::to_string(concurrency_num + ii);
    }

    uint64_t all_begin_time = GetCurTimestampMs();
    for (uint32_t ii = 0; ii < try_num; ++ii) {
      std::vector<std::shared_ptr<unifex::task<void>>> tasks(concurrency_num);
      for (auto& request_msg : request_msgs) {
        tasks.emplace_back(std::make_shared<unifex::task<void>>(unifex::co_invoke([&proxy_ptr, &request_msg, &successed_num, &total_time]() -> unifex::task<void> {
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

          co_return;
        })));
      }
      co_await when_all_vec<decltype(tasks), concurrency_num>(tasks);
    }
    uint64_t all_end_time = GetCurTimestampMs();

    printf("all done... succ: %d, timecost(ms): %llu, average timecost(ms): %llu\n", static_cast<int>(successed_num), all_end_time - all_begin_time, static_cast<uint64_t>(total_time) / try_num / concurrency_num);

    co_return;
  };

  unifex::sync_wait(co_work());

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();

  return 0;
}
