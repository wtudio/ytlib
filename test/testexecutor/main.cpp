#include <chrono>
#include <iostream>

#include <unifex/async_mutex.hpp>
#include <unifex/coroutine.hpp>
#include <unifex/inline_scheduler.hpp>
#include <unifex/just.hpp>
#include <unifex/on.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/single_thread_context.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/then.hpp>
#include <unifex/typed_via.hpp>
#include <unifex/via.hpp>
#include <unifex/when_all.hpp>

#include "ytlib/execution/boost_asio_executor.hpp"
#include "ytlib/execution/boost_fiber_executor.hpp"
#include "ytlib/misc/misc_macro.h"

#include "test1.hpp"
#include "test2.hpp"

void Test6() {
  ytlib::AsioContext asio_ctx(std::make_shared<ytlib::AsioExecutor>(2));     // asio ctx
  ytlib::FiberContext fiber_ctx(std::make_shared<ytlib::FiberExecutor>(2));  // fiber ctx

  test1::MyScheduler inline_sche;  // inline sche

  auto work1 =
      unifex::schedule(inline_sche) |  // -----------初始使用inline execute-----------
      unifex::then([]() {
        // 在inline execute下执行一些任务
        DBG_PRINT("[run in thread %llu]step 1 run in inline execute.", ytlib::GetThreadId());
      }) |
      unifex::typed_via(asio_ctx.get_scheduler()) |  // -----------切换到asio execute-----------
      unifex::then([]() {
        // 在asio ctx下执行一些任务
        DBG_PRINT("[run in thread %llu]step 2 run in asio execute.", ytlib::GetThreadId());
      }) |
      unifex::typed_via(fiber_ctx.get_scheduler()) |  // -----------切换到fiber execute-----------
      unifex::then([]() {
        // 在fiber  ctx下执行一些任务
        DBG_PRINT("[run in thread %llu]step 3-1 run in fiber execute.", ytlib::GetThreadId());
        boost::this_fiber::sleep_for(std::chrono::milliseconds(500));
        DBG_PRINT("[run in thread %llu]step 3-2 run in fiber execute.", ytlib::GetThreadId());
      });

  // (1)和(2)等效
  unifex::sync_wait(work1);     // (1)
  work1 | unifex::sync_wait();  // (2)

  // ------------------------------------------------

  auto work2 = [&]() -> unifex::task<void> {
    // 初始时使用外部调用时的ctx

    // -----------切换到inline execute-----------
    co_await unifex::schedule(inline_sche);
    // 在inline execute下执行一些任务
    DBG_PRINT("[run in thread %llu]step 1 run in inline execute.", ytlib::GetThreadId());

    // -----------切换到asio execute-----------
    co_await unifex::schedule(asio_ctx.get_scheduler());
    // 在asio ctx下执行一些任务
    DBG_PRINT("[run in thread %llu]step 2 run in asio execute.", ytlib::GetThreadId());

    // -----------切换到fiber execute-----------
    co_await unifex::schedule(fiber_ctx.get_scheduler());
    // 在fiber  ctx下执行一些任务
    DBG_PRINT("[run in thread %llu]step 3-1 run in fiber execute.", ytlib::GetThreadId());
    boost::this_fiber::sleep_for(std::chrono::milliseconds(500));
    DBG_PRINT("[run in thread %llu]step 3-2 run in fiber execute.", ytlib::GetThreadId());

    co_return;
  };

  unifex::sync_wait(work2());
}

int32_t main(int32_t argc, char** argv) {
  // 注意：只能有一个FiberExecutor

  test1::Test1();

  test2::Test2();

  Test6();

  return 0;
}
