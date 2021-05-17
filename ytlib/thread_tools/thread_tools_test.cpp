#include <gtest/gtest.h>

#include <atomic>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <thread>

#include "block_queue.hpp"
#include "channel.hpp"
#include "thread_id.hpp"
#include "ytlib/misc/misc_macro.h"

class TestObj {
 public:
  TestObj() {
    id = gid++;
    DBG_PRINT("create obj %d", id);
  }
  TestObj(const TestObj &obj) {
    id = gid++;
    DBG_PRINT("create obj %d from %d", id, obj.id);
  }
  TestObj &operator=(const TestObj &obj) {
    DBG_PRINT(" = obj %d to %d", obj.id, id);
    return *this;
  }
  TestObj(TestObj &&obj) {
    id = gid++;
    DBG_PRINT("move obj %d to %d", obj.id, id);
  }
  TestObj &operator=(const TestObj &&obj) {
    DBG_PRINT("move = obj %d to %d", obj.id, id);
    return *this;
  }
  ~TestObj() {
    DBG_PRINT("del obj %d", id);
  }
  uint32_t id;
  uint32_t tmp;
  static uint32_t gid;
};

uint32_t TestObj::gid = 0;

// 测试Channel
TEST(THREAD_TOOLS_TEST, Channel_BASE) {
  using TestChannel = ytlib::Channel<TestObj>;

  std::atomic<uint32_t> ct = 0;

  auto f = [&](TestObj &&obj) {
    DBG_PRINT("handle obj %d", obj.id);
    obj.tmp = obj.id;
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
  using BckQueue = ytlib::BlockQueue<TestObj>;
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
  using BckQueue = ytlib::BlockQueue<TestObj>;
  TestObj::gid = 0;
  BckQueue qu(100);

  std::atomic<uint32_t> ct = 0;

  auto f = [&](TestObj &&obj) {
    DBG_PRINT("handle obj %d", obj.id);
    ++ct;
  };

  std::thread t1([&] {
    qu.Enqueue(TestObj());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    qu.Enqueue(TestObj());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    qu.Stop();
  });

  std::thread t2([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

// 测试ThreadIdTool
TEST(THREAD_TOOLS_TEST, ThreadIdTool) {
  using ytlib::ThreadIdTool;

  uint64_t tid = ytlib::GetThreadId();
  for (uint32_t ii = 0; ii < 100; ++ii) {
    ASSERT_EQ(ytlib::GetThreadId(), tid);
  }

  const uint32_t thread_num = 10;
  std::mutex mu;
  std::map<uint32_t, uint64_t> thread_id_map;

  std::list<std::thread> threads;
  for (uint32_t ii = 0; ii < thread_num; ++ii) {
    threads.emplace(threads.end(), [&, ii] {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      uint64_t tid = ytlib::GetThreadId();
      for (uint32_t ii = 0; ii < 100; ++ii) {
        ASSERT_EQ(ytlib::GetThreadId(), tid);
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