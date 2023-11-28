#include <chrono>
#include <csignal>
#include <string>
#include <thread>

#include <boost/asio.hpp>
#include <exec/static_thread_pool.hpp>
#include <unifex/execute.hpp>
#include <unifex/static_thread_pool.hpp>
#include "tbb/concurrent_queue.h"
#include "ytlib/boost_tools_asio/asio_tools.hpp"
#include "ytlib/execution/boost_asio_executor.hpp"
#include "ytlib/execution/boost_fiber_executor.hpp"
#include "ytlib/function/function.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/block_queue.hpp"
#include "ytlib/thread/channel.hpp"

using namespace std::chrono_literals;

constexpr uint32_t thread_num = 4;

constexpr uint32_t loop_count = 10000000;
std::atomic_uint32_t g_val;

void TestTask(uint32_t val) {
  for (uint32_t ii = 0; ii < 2000; ++ii) {
    val += val * ii / 687 + 123;
    val *= val;
  }
  g_val += val;
}

// 测试投递开销、运行开销
// thread_pool这些只测试投递开销

void TestAsio() {
  std::chrono::steady_clock::time_point start_time;
  g_val.store(0);

  {
    ytlib::AsioExecutor asio_sys(thread_num);
    asio_sys.Start();

    std::this_thread::sleep_for(100ms);

    start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      boost::asio::post(*asio_sys.IO(), [ii]() {
        TestTask(ii);
      });
    }

    auto t = std::chrono::steady_clock::now() - start_time;
    printf("TestAsio post time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());

    asio_sys.Stop();
    asio_sys.Join();
  }

  auto t = std::chrono::steady_clock::now() - start_time;
  printf("TestAsio cal result: %u, cal time : %lu ms\n",
         g_val.load(), std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
}

void TestStdexec() {
  std::chrono::steady_clock::time_point start_time;
  g_val.store(0);

  {
    exec::static_thread_pool thread_pool(thread_num);
    auto sche = thread_pool.get_scheduler();

    std::this_thread::sleep_for(100ms);

    start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      stdexec::execute(sche, [ii]() {
        TestTask(ii);
      });
    }

    auto t = std::chrono::steady_clock::now() - start_time;
    printf("TestStdexec post time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
  }

  auto t = std::chrono::steady_clock::now() - start_time;
  printf("TestStdexec cal result: %u, cal time : %lu ms\n",
         g_val.load(), std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
}

void TestLibunifex() {
  std::chrono::steady_clock::time_point start_time;
  g_val.store(0);

  {
    unifex::static_thread_pool thread_pool(thread_num);
    auto sche = thread_pool.get_scheduler();

    std::this_thread::sleep_for(100ms);

    start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      unifex::execute(sche, [ii]() {
        TestTask(ii);
      });
    }

    auto t = std::chrono::steady_clock::now() - start_time;
    printf("TestLibunifex time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
  }

  auto t = std::chrono::steady_clock::now() - start_time;
  printf("TestLibunifex cal result: %u, cal time : %lu ms\n",
         g_val.load(), std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
}

void TestYtlibChannel() {
  std::chrono::steady_clock::time_point start_time;
  g_val.store(0);

  {
    ytlib::Channel<uint32_t> ch;
    ch.Init([](uint32_t val) { TestTask(val); }, thread_num);
    ch.StartProcess();

    std::this_thread::sleep_for(100ms);

    start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      ch.Enqueue(ii);
    }

    auto t = std::chrono::steady_clock::now() - start_time;
    printf("TestYtlibChannel post time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
  }

  auto t = std::chrono::steady_clock::now() - start_time;
  printf("TestYtlibChannel cal result: %u, cal time : %lu ms\n",
         g_val.load(), std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
}

class TbbChannel {
 public:
  using TaskType = ytlib::Function<void()>;

  explicit TbbChannel(uint32_t thread_num = 1)
      : thread_num_(thread_num) {}

  ~TbbChannel() {
    try {
      Stop();
      Join();
    } catch (const std::exception &e) {
      DBG_PRINT("TbbChannel destruct get exception, %s", e.what());
    }
  };

  // 启动线程
  void Start() {
    for (uint32_t ii = 0; ii < thread_num_; ++ii) {
      threads_.emplace_back([this]() {
        TaskType task;
        while (true) {
          while (qu_.try_pop(task)) task();

          if (!running_flag_.load()) return;

          sig_flag_.wait(false);
        };
      });
    }
  }

  // 等待任务全部执行完成后推出
  void Stop() {
    running_flag_.store(false);

    sig_flag_.store(true);
    sig_flag_.notify_all();
  }

  void Join() {
    for (auto itr = threads_.begin(); itr != threads_.end();) {
      if (itr->joinable())
        itr->join();
      threads_.erase(itr++);
    }
  }

  template <typename... Args>
  void Execute(Args &&...args) {
    qu_.emplace(std::forward<Args>(args)...);
    sig_flag_.store(true);
    sig_flag_.notify_one();
  }

 private:
  tbb::concurrent_queue<TaskType> qu_;
  std::atomic_bool sig_flag_ = false;
  std::atomic_bool running_flag_ = true;
  const uint32_t thread_num_;
  std::list<std::thread> threads_;
};

void TestTbb() {
  std::chrono::steady_clock::time_point start_time;
  g_val.store(0);

  {
    TbbChannel ch(thread_num);
    ch.Start();

    std::this_thread::sleep_for(100ms);

    start_time = std::chrono::steady_clock::now();

    for (uint32_t ii = 0; ii < loop_count; ++ii) {
      ch.Execute([ii]() {
        TestTask(ii);
      });
    }

    auto t = std::chrono::steady_clock::now() - start_time;
    printf("TestTbb post time : %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(t).count());

    ch.Stop();
    ch.Join();
  }

  auto t = std::chrono::steady_clock::now() - start_time;
  printf("TestTbb cal result: %u, cal time : %lu ms\n",
         g_val.load(), std::chrono::duration_cast<std::chrono::milliseconds>(t).count());
}

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  std::cout << "bbb " << std::this_thread::get_id() << std::endl;
  exit(signum);
}

int32_t main(int32_t argc, char **argv) {
  std::cout << "aaa " << std::this_thread::get_id() << std::endl;

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  double a = 0;

  for (uint64_t ii = 0; ii < 100000000000; ++ii) {
    for (uint64_t jj = 0; jj < 100000000000; ++jj) {
      for (uint64_t kk = 0; kk < 100000000000; ++kk) {
        a += ii;
        a *= 1.23695;
        a -= jj;
        a /= 0.69864;
        a *= kk;
      }
    }
  }
  std::cout << a << std::endl;
  std::cout << "ccc " << std::this_thread::get_id() << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(10));

  for (uint32_t ii = 0; ii < 5; ++ii) {
    printf("loop %u-----------------------------\n", ii);

    TestAsio();

    TestStdexec();

    TestLibunifex();

    TestYtlibChannel();

    TestTbb();

    printf("\n");
  }

  return 0;
}
