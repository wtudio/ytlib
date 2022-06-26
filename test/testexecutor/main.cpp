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

#include "ytlib/misc/misc_macro.h"

#include "test1.hpp"
#include "test10.hpp"
#include "test2.hpp"
#include "test3.hpp"
#include "test4.hpp"
#include "test9.hpp"

void Test6() {
  // unifex::inline_scheduler sche0;
  // unifex::single_thread_context ctx1;
  // unifex::single_thread_context ctx2;

  test3::MyContext asio_ctx(2);   // asio ctx
  test4::MyContext fiber_ctx(2);  // fiber ctx

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

struct MyReceiver {
  void set_value() noexcept {
    DBG_PRINT("[run in thread %llu]MyReceiver set_value.", ytlib::GetThreadId());
  }

  [[noreturn]] void set_error(std::exception_ptr) noexcept {
    DBG_PRINT("[run in thread %llu]MyReceiver set_error.", ytlib::GetThreadId());
    std::terminate();
  }

  void set_done() noexcept {
    DBG_PRINT("[run in thread %llu]MyReceiver set_done.", ytlib::GetThreadId());
  }
};

void Test7() {
  test3::MyContext asio_ctx(2);   // asio ctx
  test4::MyContext fiber_ctx(2);  // fiber ctx

  boost::fibers::promise<int> fiber_promise;
  boost::fibers::future<int> fiber_future = fiber_promise.get_future();

  auto fiber_work = [&]() -> unifex::task<void> {
    co_await unifex::schedule(fiber_ctx.get_scheduler());

    // 模拟进入rpc server handle中
    auto handle_work = [&]() -> unifex::task<void> {
      // 在fiber  ctx下执行一些任务
      DBG_PRINT("[run in thread %llu]step 1-1 run in fiber execute.", ytlib::GetThreadId());
      boost::this_fiber::sleep_for(std::chrono::milliseconds(500));
      DBG_PRINT("[run in thread %llu]step 1-2 run in fiber execute.", ytlib::GetThreadId());

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

      fiber_promise.set_value(42);

      co_return;
    };

    MyReceiver my_receiver;
    auto op = unifex::connect(handle_work(), my_receiver);
    unifex::start(op);

    int ret = fiber_future.get();
    DBG_PRINT("[run in thread %llu]step 4 run in fiber execute. ret = %d", ytlib::GetThreadId(), ret);

    co_return;
  };

  unifex::sync_wait(fiber_work());
}

int32_t main(int32_t argc, char** argv) {
  // 注意：只能有一个FiberExecutor

  // test1::Test1();

  // test2::Test2();

  // test3::Test3();

  // test4::Test4();

  Test6();

  // Test7();

  // test9::Test9();

  // test10::Test10();

  return 0;
}
