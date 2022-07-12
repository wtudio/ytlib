#include <gtest/gtest.h>

#include "boost_asio_executor.hpp"
#include "boost_fiber_executor.hpp"
#include "execution_tools.hpp"

#include <unifex/async_mutex.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/timed_single_thread_context.hpp>
#include <unifex/when_all.hpp>

namespace ytlib {

TEST(EXECUTION_TEST, StartDetached) {
  unifex::timed_single_thread_context ctx;
  uint32_t n = 0;

  auto work = [&]() -> unifex::task<int> {
    n = 1;
    co_await unifex::schedule_after(ctx.get_scheduler(), std::chrono::milliseconds(50));
    n = 2;
    co_return 42;
  };

  StartDetached(work());
  EXPECT_EQ(n, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(n, 2);
}

// 模拟异步请求
void AsyncSendRecv(uint32_t in, const std::function<void(uint32_t)> &callback) {
  std::thread t([in, callback]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    callback(in + 1);
  });
  t.detach();
}

void AsyncSendRecv2(uint32_t in, const std::function<void(uint32_t, std::string &&)> &callback) {
  std::thread t([in, callback]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    callback(in + 1, std::string("abc"));
  });
  t.detach();
}

TEST(EXECUTION_TEST, AsyncWrapper) {
  // single ret
  {
    auto work = []() -> unifex::task<int> {
      int ret = co_await AsyncWrapper<int>(std::bind(AsyncSendRecv, 41, std::placeholders::_1));
      co_return ret;
    };

    auto ret = unifex::sync_wait(work());
    EXPECT_EQ(*ret, 42);
  }

  // multiple ret
  {
    auto work = []() -> unifex::task<std::tuple<int, std::string>> {
      auto [ret_code, ret_str] = co_await AsyncWrapper<int, std::string>(std::bind(AsyncSendRecv2, 41, std::placeholders::_1));
      co_return std::make_tuple(ret_code, ret_str);
    };

    auto ret = unifex::sync_wait(work());
    EXPECT_EQ(std::get<0>(*ret), 42);
    EXPECT_STREQ(std::get<1>(*ret).c_str(), "abc");
  }
}

TEST(EXECUTION_TEST, AsioContext) {
  auto asio_sys_ptr = std::make_shared<AsioExecutor>(2);
  AsioContext asio_ctx(asio_sys_ptr);
  asio_sys_ptr->Start();

  // test execute
  {
    uint32_t n = 0;
    unifex::execute(asio_ctx.get_scheduler(), [&n]() {
      DBG_PRINT("[run in thread %llu]hello asio execute().", ytlib::GetThreadId());
      n = 42;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 42);
  }

  // test coro
  {
    auto work = [&]() -> unifex::task<int> {
      co_await unifex::schedule(asio_ctx.get_scheduler());
      co_return 42;
    };

    auto ret = unifex::sync_wait(work());
    EXPECT_EQ(*ret, 42);
  }

  // test coro timer
  {
    uint32_t n = 0;
    auto work = [&]() -> unifex::task<void> {
      co_await unifex::schedule(asio_ctx.get_scheduler());
      ++n;
      co_await unifex::schedule_after(asio_ctx.get_scheduler(), std::chrono::milliseconds(100));
      ++n;
      co_await unifex::schedule_at(asio_ctx.get_scheduler(), std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
      ++n;
    };

    StartDetached(work());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(n, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 3);
  }

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();
}

TEST(EXECUTION_TEST, FiberContext) {
  auto fiber_sys_ptr = std::make_shared<FiberExecutor>(2);
  FiberContext fiber_ctx(fiber_sys_ptr);
  fiber_sys_ptr->Start();

  // test execute
  {
    uint32_t n = 0;
    unifex::execute(fiber_ctx.get_scheduler(), [&n]() {
      DBG_PRINT("[run in thread %llu]hello fiber execute().", ytlib::GetThreadId());
      n = 42;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 42);
  }

  // test coro
  {
    auto work = [&]() -> unifex::task<int> {
      co_await unifex::schedule(fiber_ctx.get_scheduler());
      co_return 42;
    };

    auto ret = unifex::sync_wait(work());
    EXPECT_EQ(*ret, 42);
  }

  // test coro in fiber
  {
    uint32_t n = 0;
    fiber_sys_ptr->Post([&n]() {
      unifex::timed_single_thread_context ctx;

      auto work = [&]() -> unifex::task<int> {
        co_await unifex::schedule_after(ctx.get_scheduler(), std::chrono::milliseconds(50));
        co_return 42;
      };

      auto ret = FiberWait(work());
      n = *ret;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 42);
  }

  // test coro timer
  {
    uint32_t n = 0;
    auto work = [&]() -> unifex::task<void> {
      co_await unifex::schedule(fiber_ctx.get_scheduler());
      ++n;
      co_await unifex::schedule_after(fiber_ctx.get_scheduler(), std::chrono::milliseconds(100));
      ++n;
      co_await unifex::schedule_at(fiber_ctx.get_scheduler(), std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
      ++n;
    };

    StartDetached(work());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(n, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 3);
  }

  fiber_sys_ptr->Stop();
  fiber_sys_ptr->Join();
}

TEST(EXECUTION_TEST, async_mutex) {
  unifex::async_mutex mutex;

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(2);
  AsioContext asio_ctx(asio_sys_ptr);
  asio_sys_ptr->Start();

  unifex::timed_single_thread_context thread_ctx;

  int shared_state = 0;

  auto asio_ctx_task = [&]() -> unifex::task<void> {
    for (int i = 0; i < 10; ++i) {
      co_await mutex.async_lock();
      co_await unifex::schedule(asio_ctx.get_scheduler());
      ++shared_state;
      mutex.unlock();
    }
    co_return;
  };

  auto thread_ctx_task = [&]() -> unifex::task<void> {
    for (int i = 0; i < 10; ++i) {
      co_await mutex.async_lock();
      co_await unifex::schedule(thread_ctx.get_scheduler());
      ++shared_state;
      mutex.unlock();
    }
    co_return;
  };

  unifex::sync_wait(unifex::when_all(asio_ctx_task(), thread_ctx_task()));

  EXPECT_EQ(shared_state, 20);

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();
}

}  // namespace ytlib
