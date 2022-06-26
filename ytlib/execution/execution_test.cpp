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

TEST(EXECUTION_TEST, AsioContext) {
  auto asio_sys_ptr = std::make_shared<AsioExecutor>(2);
  AsioContext asio_ctx(asio_sys_ptr);
  asio_sys_ptr->Start();

  uint32_t n = 0;
  unifex::execute(asio_ctx.get_scheduler(), [&n]() {
    DBG_PRINT("[run in thread %llu]hello asio execute().", ytlib::GetThreadId());
    n = 42;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();

  EXPECT_EQ(n, 42);
}

TEST(EXECUTION_TEST, FiberContext) {
  auto fiber_sys_ptr = std::make_shared<FiberExecutor>(2);
  FiberContext fiber_ctx(fiber_sys_ptr);
  fiber_sys_ptr->Start();

  uint32_t n = 0;
  unifex::execute(fiber_ctx.get_scheduler(), [&n]() {
    DBG_PRINT("[run in thread %llu]hello fiber execute().", ytlib::GetThreadId());
    n = 42;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  fiber_sys_ptr->Stop();
  fiber_sys_ptr->Join();

  EXPECT_EQ(n, 42);
}

TEST(EXECUTION_TEST, async_mutex) {
  unifex::async_mutex mutex;

  auto asio_sys_ptr = std::make_shared<AsioExecutor>(2);
  AsioContext asio_ctx(asio_sys_ptr);
  asio_sys_ptr->Start();

  auto fiber_sys_ptr = std::make_shared<FiberExecutor>(2);
  FiberContext fiber_ctx(fiber_sys_ptr);
  fiber_sys_ptr->Start();

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

  auto fiber_ctx_task = [&]() -> unifex::task<void> {
    for (int i = 0; i < 10; ++i) {
      co_await mutex.async_lock();
      co_await unifex::schedule(fiber_ctx.get_scheduler());
      ++shared_state;
      mutex.unlock();
    }
    co_return;
  };

  unifex::sync_wait(unifex::when_all(asio_ctx_task(), fiber_ctx_task()));

  EXPECT_EQ(shared_state, 20);

  asio_sys_ptr->Stop();
  asio_sys_ptr->Join();

  fiber_sys_ptr->Stop();
  fiber_sys_ptr->Join();
}

// todo: add schedule after/at
TEST(EXECUTION_TEST, misc) {
  auto work = [&]() -> unifex::task<uint32_t> {
    co_return 42;
  };

  auto work_ret = unifex::sync_wait(work());
  EXPECT_EQ(*work_ret, 42);
}

}  // namespace ytlib
