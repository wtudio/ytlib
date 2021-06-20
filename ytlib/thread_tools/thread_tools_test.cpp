#include <gtest/gtest.h>

#include <atomic>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <thread>

#include "block_queue.hpp"
#include "channel.hpp"
#include "coroutine_tools.hpp"
#include "signal.hpp"
#include "thread_id.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread_tools/thread_id.hpp"

namespace ytlib {

class TestObj {
 public:
  TestObj() {
    id = gid++;
    DBG_PRINT("[%lld]create obj %d", ytlib::GetThreadId(), id);
    data = "";
  }
  TestObj(const TestObj &obj) : data(obj.data) {
    id = gid++;
    DBG_PRINT("[%lld]create obj %d from %d by copy", ytlib::GetThreadId(), id, obj.id);
  }
  TestObj &operator=(const TestObj &obj) {
    DBG_PRINT("[%lld]copy obj %d to %d", ytlib::GetThreadId(), obj.id, id);
    data = obj.data;
    return *this;
  }
  TestObj(TestObj &&obj) : data(std::move(obj.data)) {
    id = gid++;
    DBG_PRINT("[%lld]create obj %d from %d by move", ytlib::GetThreadId(), id, obj.id);
  }
  TestObj &operator=(const TestObj &&obj) {
    DBG_PRINT("[%lld]move obj %d to %d", ytlib::GetThreadId(), obj.id, id);
    data = std::move(obj.data);
    return *this;
  }
  ~TestObj() {
    DBG_PRINT("[%lld]del obj %d", ytlib::GetThreadId(), id);
  }
  uint32_t id;
  std::string data;
  static uint32_t gid;
};

uint32_t TestObj::gid = 0;

// 测试Channel
TEST(THREAD_TOOLS_TEST, Channel_BASE) {
  using TestChannel = Channel<TestObj>;

  std::atomic<uint32_t> ct = 0;

  auto f = [&](TestObj &&obj) {
    DBG_PRINT("[%lld]handle obj %d", obj.id);
    ++ct;
  };

  TestChannel ch;
  ch.Init(f, 2);
  ch.StartProcess();
  uint32_t obj_num = 10;
  for (uint32_t ii = 0; ii < obj_num; ++ii) {
    ch.Enqueue(TestObj());
  }
  ch.StopProcess();
  ASSERT_EQ(ch.Count(), 0);
  ASSERT_EQ(ct, obj_num);
}

// 测试BlockQueue基础同步操作
TEST(THREAD_TOOLS_TEST, BlockQueue_BASE) {
  using BckQueue = BlockQueue<TestObj>;
  TestObj::gid = 0;
  uint32_t n = 5;
  BckQueue qu(n);
  ASSERT_EQ(qu.GetMaxCount(), n);
  ASSERT_EQ(qu.Count(), 0);

  TestObj obj1;
  qu.Enqueue(std::move(obj1));
  ASSERT_EQ(qu.Count(), 1);

  TestObj obj2;
  ASSERT_EQ(qu.Dequeue(obj2), true);
  ASSERT_EQ(qu.Count(), 0);

  ASSERT_EQ(TestObj::gid, 3);

  ASSERT_EQ(qu.Dequeue(obj2), false);

  ASSERT_EQ(TestObj::gid, 3);

  for (uint32_t ii = 0; ii < n; ++ii) {
    TestObj obj_tmp;
    ASSERT_EQ(qu.Enqueue(obj_tmp), true);
  }

  TestObj obj_tmp;
  ASSERT_EQ(qu.Enqueue(obj_tmp), false);
  ASSERT_EQ(qu.Count(), qu.GetMaxCount());

  ASSERT_EQ(qu.BlockDequeue(obj2), true);
  ASSERT_EQ(qu.Count(), qu.GetMaxCount() - 1);

  qu.Clear();
  ASSERT_EQ(qu.Count(), 0);
}

// 测试BlockQueue异步操作
TEST(THREAD_TOOLS_TEST, BlockQueue_ANYSC) {
  using BckQueue = BlockQueue<TestObj>;
  TestObj::gid = 0;
  BckQueue qu(100);

  std::atomic<uint32_t> ct = 0;

  auto f = [&](TestObj &&obj) {
    DBG_PRINT("handle obj %d", obj.id);
    ++ct;
  };

  std::thread t1([&] {
    qu.Enqueue(TestObj());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    qu.Enqueue(TestObj());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    qu.Stop();
  });

  std::thread t2([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    TestObj obj;
    ASSERT_EQ(qu.BlockDequeue(obj), true);
    ASSERT_EQ(TestObj::gid, 3);
    ASSERT_EQ(qu.BlockDequeue(f), true);
    ASSERT_EQ(qu.BlockDequeue(obj), false);
    ASSERT_EQ(qu.BlockDequeue(f), false);
  });

  t1.join();
  t2.join();
  ASSERT_EQ(ct, 1);
}

// 测试LightSignal
TEST(THREAD_TOOLS_TEST, LightSignal_BASE) {
  LightSignal s;
  uint32_t i = 0;

  std::thread t1([&] {
    ASSERT_EQ(i, 0);
    s.wait();  // node1
    ASSERT_EQ(i, 1);
  });

  std::thread t2([&] {
    ASSERT_EQ(i, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    i = 1;
    s.notify();  // node1
  });

  t1.join();
  t2.join();
}

// 测试LightSignalAtomic
TEST(THREAD_TOOLS_TEST, LightSignalAtomic_BASE) {
  LightSignalAtomic s;
  uint32_t i = 0;

  std::thread t1([&] {
    ASSERT_EQ(i, 0);
    s.wait();  // node1
    ASSERT_EQ(i, 1);
  });

  std::thread t2([&] {
    ASSERT_EQ(i, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    i = 1;
    s.notify();  // node1
  });

  t1.join();
  t2.join();
}

// 测试ThreadIdTool
TEST(THREAD_TOOLS_TEST, ThreadIdTool_BASE) {
  uint64_t tid = GetThreadId();
  for (uint32_t ii = 0; ii < 100; ++ii) {
    ASSERT_EQ(GetThreadId(), tid);
  }

  const uint32_t thread_num = 10;
  std::mutex mu;
  std::map<uint32_t, uint64_t> thread_id_map;

  std::list<std::thread> threads;
  for (uint32_t ii = 0; ii < thread_num; ++ii) {
    threads.emplace(threads.end(), [&, ii] {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      uint64_t tid = GetThreadId();
      for (uint32_t ii = 0; ii < 100; ++ii) {
        ASSERT_EQ(GetThreadId(), tid);
      }

      mu.lock();
      thread_id_map.emplace(ii, tid);
      mu.unlock();
    });
  }

  for (auto itr = threads.begin(); itr != threads.end(); itr++) {
    itr->join();
  }

  std::set<uint64_t> thread_id_set;
  for (auto &itr : thread_id_map) {
    ASSERT_EQ(thread_id_set.find(itr.second) == thread_id_set.end(), true);
    thread_id_set.insert(itr.second);
  }
}

// 模拟异步请求
void AsyncSendRecv(const TestObj &in_buf, std::function<void(TestObj &&)> callback) {
  std::thread t([&in_buf, callback]() {
    std::cout << "aaaaa" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    TestObj out_buf;  // 3、7
    out_buf.data = in_buf.data + "-echo";
    callback(std::move(out_buf));
    std::cout << "bbbbbb" << std::endl;
  });
  t.detach();
}

TEST(THREAD_TOOLS_TEST, coroutine_BASE) {
  TestObj::gid = 0;

  TestObj buf;  // 0
  buf.data = "abcd";

  auto sched = [&buf]() -> CoroSched<TestObj> {
    // CoroSched.ret_ 1
    std::cout << "start coro" << std::endl;

    // Awaitable.re_ 2
    // ret_buf 4
    TestObj ret_buf = co_await Awaitable<TestObj>([&buf](std::function<void(TestObj &&)> cb) {
      AsyncSendRecv(buf, cb);
    });
    std::cout << "coro 1" << std::endl;
    co_yield ret_buf;  // 4->1  node1

    std::cout << "coro 2" << std::endl;

    // Awaitable.re_ 6
    // ret_buf2 8
    TestObj ret_buf2 = co_await Awaitable<TestObj>([&ret_buf](std::function<void(TestObj &&)> cb) {
      AsyncSendRecv(ret_buf, cb);
    });

    std::cout << "coro 3" << std::endl;

    co_yield ret_buf2;  // 8->1 node2

    std::cout << "coro 4" << std::endl;

    co_yield std::move(ret_buf);  // 4->1 node3

    std::cout << "coro 5" << std::endl;

    co_return ret_buf2;  // node4
  }();

  // run
  std::cout << "ttttt" << std::endl;
  // 5
  TestObj out_buf = sched.Get();  // node1
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo");

  // not run
  out_buf = sched.Get();
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo");

  // run
  sched.Resume();         // node1 start
  out_buf = sched.Get();  // node2
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo-echo");

  // run
  sched.Resume();         // node2 start
  out_buf = sched.Get();  // node3
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo");

  // run
  sched.Resume();         // node3 start
  out_buf = sched.Get();  // node4
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo-echo");
}

}  // namespace ytlib
