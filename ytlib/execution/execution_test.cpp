#include <gtest/gtest.h>

#include "ytlib/boost_tools_asio/asio_tools.hpp"

#include "boost_asio_executor.hpp"
#include "boost_fiber_executor.hpp"
#include "execution_tools.hpp"

#include <unifex/async_manual_reset_event.hpp>
#include <unifex/async_mutex.hpp>
#include <unifex/async_scope.hpp>
#include <unifex/inline_scheduler.hpp>
#include <unifex/on.hpp>
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

  StartDetached(work(), [](int) {});
  EXPECT_EQ(n, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(n, 2);

  uint32_t m = 0;
  StartDetached(work(), [&m](int ret) { m = ret; });
  EXPECT_EQ(m, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(m, 42);
}

// 模拟异步请求
void AsyncSendRecv(uint32_t in, const std::function<void(uint32_t)> &callback) {
  std::thread t([in, callback]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    callback(in + 1);
  });
  t.detach();
}

void AsyncSendRecv2(uint32_t in, const std::function<void(uint32_t, std::string &&)> &callback) {
  std::thread t([in, callback]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
  AsioContext asio_ctx(asio_sys_ptr->IO());
  asio_sys_ptr->Start();

  // test execute
  {
    uint32_t n = 0;
    unifex::execute(asio_ctx.GetScheduler(), [&n]() {
      DBG_PRINT("[run in thread %llu]hello asio execute().", ytlib::GetThreadId());
      n = 42;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 42);
  }

  // test strand
  {
    uint32_t n = 0;
    uint32_t ct = 1000;

    auto asio_strand_scheduler = asio_ctx.GetStrandScheduler();
    for (uint32_t ii = 0; ii < ct; ++ii) {
      unifex::execute(asio_strand_scheduler, [&n]() {
        // DBG_PRINT("[run in thread %llu]hello asio strand execute().", ytlib::GetThreadId());
        n++;
      });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, ct);
  }

  // test coro
  {
    auto work = [&]() -> unifex::task<int> {
      co_await unifex::schedule(asio_ctx.GetScheduler());
      co_return 42;
    };

    auto ret = unifex::sync_wait(work());
    EXPECT_EQ(*ret, 42);
  }

  // test coro timer
  {
    uint32_t n = 0;
    auto work = [&]() -> unifex::task<void> {
      co_await unifex::schedule(asio_ctx.GetScheduler());
      ++n;
      co_await unifex::schedule_after(asio_ctx.GetScheduler(), std::chrono::milliseconds(100));
      ++n;
      co_await unifex::schedule_at(asio_ctx.GetScheduler(), std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
      ++n;
    };

    unifex::async_scope scope;
    scope.spawn(work());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(n, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 3);

    unifex::sync_wait(scope.cleanup());
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
    unifex::execute(fiber_ctx.GetScheduler(), [&n]() {
      DBG_PRINT("[run in thread %llu]hello fiber execute().", ytlib::GetThreadId());
      n = 42;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(n, 42);
  }

  // test coro
  {
    auto work = [&]() -> unifex::task<int> {
      co_await unifex::schedule(fiber_ctx.GetScheduler());
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

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(n, 42);
  }

  // test coro timer
  {
    uint32_t n = 0;
    auto work = [&]() -> unifex::task<void> {
      co_await unifex::schedule(fiber_ctx.GetScheduler());
      ++n;
      co_await unifex::schedule_after(
          fiber_ctx.GetScheduler(), std::chrono::milliseconds(400));
      ++n;
      co_await unifex::schedule_at(
          fiber_ctx.GetScheduler(),
          std::chrono::steady_clock::now() + std::chrono::milliseconds(400));
      ++n;
    };

    unifex::async_scope scope;
    scope.spawn(work());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(n, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    EXPECT_EQ(n, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    EXPECT_EQ(n, 3);

    unifex::sync_wait(scope.cleanup());
  }

  fiber_sys_ptr->Stop();
  fiber_sys_ptr->Join();
}

TEST(EXECUTION_TEST, async_mutex) {
  unifex::async_mutex mutex;

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(2);
  AsioContext asio_ctx(asio_sys_ptr->IO());
  asio_sys_ptr->Start();

  unifex::timed_single_thread_context thread_ctx;

  int shared_state = 0;

  auto asio_ctx_task = [&]() -> unifex::task<void> {
    for (int i = 0; i < 10; ++i) {
      co_await mutex.async_lock();
      co_await unifex::schedule(asio_ctx.GetScheduler());
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

TEST(EXECUTION_TEST, async_manual_reset_event) {
  unifex::async_manual_reset_event sig_1;
  unifex::async_manual_reset_event sig_2;

  int n = 0;

  auto work = [&]() -> unifex::task<void> {
    unifex::inline_scheduler sche;
    co_await unifex::on(sche, sig_1.async_wait());
    n = 42;
    co_await unifex::on(sche, sig_2.async_wait());
    n = 43;
    co_return;
  };

  unifex::async_scope scope;
  scope.spawn(work());
  EXPECT_EQ(n, 0);

  sig_1.set();
  EXPECT_EQ(n, 42);

  sig_2.set();
  EXPECT_EQ(n, 43);

  scope.spawn(work());
  EXPECT_EQ(n, 43);

  unifex::sync_wait(scope.cleanup());
}

TEST(EXECUTION_TEST, AsyncLatch) {
  uint32_t num = 100;
  AsyncLatch latch(num);
  int n = 0;

  auto work = [&]() -> unifex::task<void> {
    unifex::inline_scheduler sche;
    co_await unifex::on(sche, latch.AsyncWait());
    n = 42;
    co_return;
  };

  unifex::async_scope scope;
  scope.spawn(work());
  EXPECT_EQ(n, 0);

  for (uint32_t ii = 0; ii < (num - 1); ++ii) {
    latch.CountDown();
    EXPECT_FALSE(latch.Ready());
    EXPECT_EQ(n, 0);
  }

  latch.CountDown();
  EXPECT_TRUE(latch.Ready());
  EXPECT_EQ(n, 42);

  n = 0;
  scope.spawn(work());
  EXPECT_TRUE(latch.Ready());
  EXPECT_EQ(n, 42);

  unifex::sync_wait(scope.cleanup());
}

}  // namespace ytlib
