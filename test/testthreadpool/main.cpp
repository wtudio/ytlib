#include <chrono>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <exec/static_thread_pool.hpp>
#include <unifex/execute.hpp>
#include <unifex/static_thread_pool.hpp>
#include "ytlib/boost_tools_asio/asio_tools.hpp"
#include "ytlib/execution/boost_asio_executor.hpp"
#include "ytlib/execution/boost_fiber_executor.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/block_queue.hpp"

using namespace std;
using namespace ytlib;
using namespace std::chrono_literals;

constexpr uint32_t thread_num = 4;

constexpr uint32_t loop_count = 10000000;
uint32_t result[loop_count];

void TestTask(uint32_t i) {
  uint32_t re = i * i * i / 1000 + i;
  result[i] = re;
}

// 测试投递开销、运行开销
// thread_pool这些只测试投递开销

void TestAsio() {
  AsioExecutor asio_sys(thread_num);
  asio_sys.Start();

  std::this_thread::sleep_for(100ms);

  // 投递线程
  std::thread t1([&asio_sys]() {
    auto start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      boost::asio::post(*asio_sys.IO(), [ii]() {
        TestTask(ii);
      });
    }

    auto end_time = std::chrono::steady_clock::now();
    auto t = end_time - start_time;

    printf("TestAsio time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
  });

  t1.join();

  asio_sys.Stop();
  asio_sys.Join();
}

void TestStdexec() {
  exec::static_thread_pool thread_pool(thread_num);

  std::this_thread::sleep_for(100ms);

  // 投递线程
  std::thread t1([&thread_pool]() {
    auto sche = thread_pool.get_scheduler();

    auto start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      stdexec::execute(sche, [ii]() {
        TestTask(ii);
      });
    }

    auto end_time = std::chrono::steady_clock::now();
    auto t = end_time - start_time;

    printf("TestStdexec time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
  });

  t1.join();
}

void TestLibunifex() {
  unifex::static_thread_pool thread_pool(thread_num);

  std::this_thread::sleep_for(100ms);

  // 投递线程
  std::thread t1([&thread_pool]() {
    auto sche = thread_pool.get_scheduler();

    auto start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      unifex::execute(sche, [ii]() {
        TestTask(ii);
      });
    }

    auto end_time = std::chrono::steady_clock::now();
    auto t = end_time - start_time;

    printf("TestLibunifex time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
  });

  t1.join();
}

int32_t main(int32_t argc, char **argv) {
  for (uint32_t ii = 0; ii < 5; ++ii) {
    printf("loop %u-----------------------------\n", ii);

    std::this_thread::sleep_for(100ms);
    TestAsio();

    std::this_thread::sleep_for(100ms);
    TestStdexec();

    std::this_thread::sleep_for(100ms);
    TestLibunifex();

    printf("\n");
  }

  return 0;
}
