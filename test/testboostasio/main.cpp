/**
 * @file main.cpp
 * @brief 自定义测试
 * @note 自定义相关测试
 * @author WT
 * @date 2019-07-26
 */

#include <coroutine>
#include <cstdarg>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <boost/asio/experimental/promise.hpp>

#include "ytlib/boost_tools_asio/asio_debug_tools.hpp"
#include "ytlib/boost_tools_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/misc/time.hpp"
#include "ytlib/thread/thread_id.hpp"

using namespace std;
using namespace ytlib;

boost::asio::awaitable<void> TestTimerCo1(boost::asio::steady_timer& t1, boost::asio::steady_timer& t2) {
  ASIO_DEBUG_HANDLE(TestTimerCo1);
  try {
    t1.expires_after(std::chrono::seconds(1));
    std::cerr << "thread " << std::this_thread::get_id() << " run TestTimerCo1.\n";
    co_await t1.async_wait(boost::asio::use_awaitable);
    t2.expires_after(std::chrono::seconds(123));
  } catch (const std::exception& e) {
    std::cerr << "TestTimerCo1 get exception:" << e.what() << '\n';
  }
  co_return;
}

boost::asio::awaitable<void> TestTimerCo2(boost::asio::steady_timer& t1, boost::asio::steady_timer& t2) {
  ASIO_DEBUG_HANDLE(TestTimerCo2);
  try {
    t2.expires_after(std::chrono::seconds(2));
    std::cerr << "thread " << std::this_thread::get_id() << " run TestTimerCo2.\n";
    co_await t2.async_wait(boost::asio::use_awaitable);
  } catch (const std::exception& e) {
    std::cerr << "TestTimerCo2 get exception:" << e.what() << '\n';
  }
  co_return;
}

/**
 * @brief
 * boost::asio::steady_timer 在协程1中async_wait时，如果在其他线程/协程里重新设置了expires_after，则会触发在协程1里的异常
 *
 */
void Test1() {
  uint32_t n = 2;
  boost::asio::io_context io(n);
  boost::asio::steady_timer t1(io);
  boost::asio::steady_timer t2(io);

  boost::asio::co_spawn(io, TestTimerCo1(t1, t2), boost::asio::detached);
  boost::asio::co_spawn(io, TestTimerCo2(t1, t2), boost::asio::detached);

  std::list<std::thread> threads_list;

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_list.emplace(threads_list.end(), [&io] {
      std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
      io.run();
      std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
    });
  }

  for (auto itr = threads_list.begin(); itr != threads_list.end();) {
    itr->join();
    threads_list.erase(itr++);
  }
}

boost::asio::awaitable<uint32_t> Test2Co1(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(Test2Co1);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run Test2Co1-a.\n";
    boost::asio::steady_timer t1(io);
    t1.expires_after(std::chrono::seconds(1));
    co_await t1.async_wait(boost::asio::use_awaitable);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test2Co1-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "Test2Co1 get exception:" << e.what() << '\n';
  }
  co_return 111;
}

boost::asio::awaitable<void> Test2Co2(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(Test2Co2);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run Test2Co2-a.\n";
    uint32_t co1_re = co_await boost::asio::co_spawn(io, Test2Co1(io), boost::asio::use_awaitable);
    std::cerr << "co1_re " << co1_re << " \n";
    co1_re = co_await Test2Co1(io);
    std::cerr << "co1_re " << co1_re << " \n";
    std::cerr << "thread " << std::this_thread::get_id() << " run Test2Co2-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "Test2Co2 get exception:" << e.what() << '\n';
  }
  co_return;
}

boost::asio::awaitable<void> Test2Co3(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(Test2Co3);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run Test2Co3-a.\n";
    boost::asio::co_spawn(io, Test2Co2(io), boost::asio::detached);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test2Co3-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "Test2Co3 get exception:" << e.what() << '\n';
  }
  co_return;
}

/**
 * @brief
 * 1、co_spawn创建的协程是类似于dispatch的运行方式，会先尽可能的执行下去，直到遇到co_await
 * 2、使用 co_await boost::asio::co_spawn(io, handle, boost::asio::use_awaitable) 来在协程内部等待另一个协程
 * 3、如果在io作为执行者的协程内部再执行 co_await boost::asio::co_spawn(io, handle, boost::asio::use_awaitable) 则等价于 co_await handle
 */
void Test2() {
  uint32_t n = 2;
  boost::asio::io_context io(n);

  boost::asio::co_spawn(io, Test2Co3(io), boost::asio::detached);

  // boost::asio::strand<boost::asio::io_context::executor_type> strand = boost::asio::make_strand(io);

  std::list<std::thread> threads_list;

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_list.emplace(threads_list.end(), [&io] {
      std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
      io.run();
      std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
    });
  }

  for (auto itr = threads_list.begin(); itr != threads_list.end();) {
    itr->join();
    threads_list.erase(itr++);
  }
}

boost::asio::awaitable<void> Test3Co1(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(Test3Co1);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co1-a.\n";
    boost::asio::steady_timer t1(io);
    t1.expires_after(std::chrono::seconds(2));
    co_await t1.async_wait(boost::asio::use_awaitable);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co1-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "Test3Co1 get exception:" << e.what() << '\n';
  }
  co_return;
}

boost::asio::awaitable<void> Test3Co2(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(Test3Co2);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co2-a.\n";
    boost::asio::steady_timer t1(io);
    t1.expires_after(std::chrono::seconds(1));
    co_await t1.async_wait(boost::asio::use_awaitable);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co2-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "Test3Co2 get exception:" << e.what() << '\n';
  }
  co_return;
}

boost::asio::awaitable<void> Test3Co3(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(Test3Co3);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co3-a.\n";

    auto Test3Co1_promise = boost::asio::co_spawn(io, Test3Co1(io), boost::asio::experimental::use_promise);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co3-b.\n";

    auto Test3Co2_promise = boost::asio::co_spawn(io, Test3Co2(io), boost::asio::experimental::use_promise);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co3-c.\n";

    co_await Test3Co1_promise.async_wait(boost::asio::use_awaitable);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co3-d.\n";

    co_await Test3Co2_promise.async_wait(boost::asio::use_awaitable);
    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co3-e.\n";

  } catch (const std::exception& e) {
    std::cerr << "Test3Co3 get exception:" << e.what() << '\n';
  }
  co_return;
}

/**
 * @brief
 * 使用use_promise可以创建一个promise，可以稍后【co_await promise.async_wait(boost::asio::use_awaitable);】，从而达到多协程并发的效果
 */
void Test3() {
  uint32_t n = 2;
  boost::asio::io_context io(n);

  boost::asio::co_spawn(io, Test3Co3(io), boost::asio::detached);

  std::list<std::thread> threads_list;

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_list.emplace(threads_list.end(), [&io] {
      std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
      io.run();
      std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
    });
  }

  for (auto itr = threads_list.begin(); itr != threads_list.end();) {
    itr->join();
    threads_list.erase(itr++);
  }
}

template <class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(std::string))
MyAsyncFunc(boost::asio::io_context& io, const std::string& instr, CompletionToken&& token) {
  return boost::asio::async_initiate<CompletionToken, void(std::string)>(
      [&io](auto completion_handler, const std::string& instr) {
        boost::asio::post(io, [&io, &instr, completion_handler{std::move(completion_handler)}]() mutable {
          std::string outstr = "echo " + instr;
          completion_handler(outstr);
        });
      },
      token,
      instr);
}

boost::asio::awaitable<void> Test4Co1(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(Test4Co1);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run Test4Co1-a.\n";

    std::string instr = "aaa";
    auto retstr = co_await MyAsyncFunc(io, instr, boost::asio::use_awaitable);
    std::cerr << "retstr: " << retstr << "\n";

    std::cerr << "thread " << std::this_thread::get_id() << " run Test3Co3-d.\n";

  } catch (const std::exception& e) {
    std::cerr << "Test4Co1 get exception:" << e.what() << '\n';
  }
  co_return;
}

/**
 * @brief 异步改协程的方式（不用timer等io）
 *
 */
void Test4() {
  uint32_t n = 2;
  boost::asio::io_context io(n);

  boost::asio::co_spawn(io, Test4Co1(io), boost::asio::detached);

  std::list<std::thread> threads_list;

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_list.emplace(threads_list.end(), [&io] {
      std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
      io.run();
      std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
    });
  }

  for (auto itr = threads_list.begin(); itr != threads_list.end();) {
    itr->join();
    threads_list.erase(itr++);
  }
}

boost::asio::awaitable<void> Test5Co1(boost::asio::io_context& io) {
  try {
    boost::asio::steady_timer t1(io);
    t1.expires_after(std::chrono::seconds(5));
    co_await t1.async_wait(boost::asio::use_awaitable);

  } catch (const std::exception& e) {
  }
  co_return;
}

/**
 * @brief 单线程跑io_context时不会有并发问题，外部线程在post/dispatch时会加锁
 *
 */
void Test5() {
  uint32_t ct = 0;
  uint32_t n = 8;

  boost::asio::io_context io(1);

  boost::asio::co_spawn(io, Test5Co1(io), boost::asio::detached);

  std::list<std::thread> threads_list;

  threads_list.emplace(threads_list.end(), [&io] {
    std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
    io.run();
    std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
  });

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_list.emplace(threads_list.end(), [&ct, &io, ii] {
      DBG_PRINT("run in thread %llu, tag %u, step 1, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
      boost::asio::dispatch(io, [&ct, ii]() {
        DBG_PRINT("run in thread %llu, tag %u, step 2, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
        for (uint32_t jj = 0; jj < 1000; ++jj) {
          ++ct;
        }
        DBG_PRINT("run in thread %llu, tag %u, step 3, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
      });
      DBG_PRINT("run in thread %llu, tag %u, step 4, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
    });
  }

  for (auto itr = threads_list.begin(); itr != threads_list.end();) {
    itr->join();
    threads_list.erase(itr++);
  }
  DBG_PRINT("ct:%u", ct);
}

boost::asio::awaitable<void> Test6Co1(boost::asio::io_context& io) {
  try {
    boost::asio::steady_timer t1(io);
    int ct = 5;
    while (--ct) {
      DBG_PRINT("run in thread %llu, timer step 1, time ms %llu", ytlib::GetThreadId(), GetCurTimestampMs());
      t1.expires_after(std::chrono::milliseconds(500));
      co_await t1.async_wait(boost::asio::use_awaitable);
      DBG_PRINT("run in thread %llu, timer step 2, time ms %llu", ytlib::GetThreadId(), GetCurTimestampMs());
    }

  } catch (const std::exception& e) {
  }
  co_return;
}

/**
 * @brief post的任务中如果耗时太久，可能导致定时器失准
 * 定时器的任务并没有更高的优先级
 */
void Test6() {
  boost::asio::io_context io(1);

  boost::asio::co_spawn(io, Test6Co1(io), boost::asio::detached);

  int n = 3;
  for (uint32_t ii = 0; ii < n; ++ii) {
    boost::asio::post(io, [ii]() {
      DBG_PRINT("run in thread %llu, tag %u, step 1, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
      std::this_thread::sleep_for(std::chrono::seconds(1));
      DBG_PRINT("run in thread %llu, tag %u, step 2, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
    });
  }

  std::list<std::thread> threads_list;

  threads_list.emplace(threads_list.end(), [&io] {
    std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
    io.run();
    std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
  });

  for (auto itr = threads_list.begin(); itr != threads_list.end();) {
    itr->join();
    threads_list.erase(itr++);
  }
}

/**
 * @brief 可以使用run_for/run_util来实现高优定时器任务
 *
 */
void Test6_1() {
  boost::asio::io_context io(1);

  int n = 3;
  for (uint32_t ii = 0; ii < n; ++ii) {
    boost::asio::post(io, [ii]() {
      DBG_PRINT("run in thread %llu, tag %u, step 1, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
      std::this_thread::sleep_for(std::chrono::seconds(1));
      DBG_PRINT("run in thread %llu, tag %u, step 2, time ms %llu", ytlib::GetThreadId(), ii, GetCurTimestampMs());
    });
  }

  std::list<std::thread> threads_list;

  threads_list.emplace(threads_list.end(), [&io] {
    std::cerr << "thread " << std::this_thread::get_id() << " start.\n";

    std::chrono::steady_clock::time_point tp_start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point tp_next = tp_start + std::chrono::milliseconds(500);
    int ct = 10;
    while (--ct) {
      io.run_until(tp_next);
      DBG_PRINT("run in thread %llu, run main update, time ms %llu", ytlib::GetThreadId(), GetCurTimestampMs());
      tp_next += std::chrono::milliseconds(500);
    }

    std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
  });

  for (auto itr = threads_list.begin(); itr != threads_list.end();) {
    itr->join();
    threads_list.erase(itr++);
  }
}

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  // Test1();

  // Test2();

  // Test3();

  // Test4();

  // Test5();

  // Test6();

  Test6_1();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  DBG_PRINT("********************end test*******************");
  return 0;
}
