#include <gtest/gtest.h>

#include <atomic>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "block_queue.hpp"
#include "channel.hpp"
#include "coroutine_tools.hpp"
#include "guid.hpp"
#include "signal.hpp"
#include "thread_id.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

TEST(THREAD_TEST, Guid) {
  // 生成mac值
  std::string mac = "testmac::abc::def";
  std::string svr_id = "testsvr";
  int thread_id = 123;
  uint32_t mac_hash = std::hash<std::string>{}(mac + svr_id + std::to_string(thread_id)) % GUID_MAC_NUM;

  GuidGener::Ins().Init(mac_hash);

  // 生成obj值
  std::string obj_name = "test_obj_name";
  uint32_t obj_hash = std::hash<std::string>{}(obj_name) % GUID_OBJ_NUM;

  // 直接生成guid
  Guid guid_last = GuidGener::Ins().GetGuid(obj_hash);

  // 获取objgener
  ObjGuidGener gener;
  gener.Init(obj_hash);

  // 用objgener生成guid
  for (int ii = 0; ii < 1000; ++ii) {
    Guid guid_cur = gener.GetGuid();
    ASSERT_GT(guid_cur.id, guid_last.id);
    guid_last = guid_cur;
  }
}

// 测试 ins 溢出时单调性 —— 历史 bug：当 ins 达到 GUID_INST_NUM 时
// 写回会被位域截断为 0，导致同秒内出现重复 guid
TEST(THREAD_TEST, Guid_InsOverflow) {
  GuidGener::Ins().Init(0);

  ObjGuidGener gener;
  gener.Init(0);

  Guid guid_last = gener.GetGuid();
  // 生成数量远大于 GUID_INST_NUM，覆盖溢出路径
  const uint32_t n = GUID_INST_NUM * 4;
  for (uint32_t ii = 0; ii < n; ++ii) {
    Guid guid_cur = gener.GetGuid();
    ASSERT_GT(guid_cur.id, guid_last.id);
    guid_last = guid_cur;
  }
}

// 测试 Init 可多次调用，且不会泄漏旧 buf
TEST(THREAD_TEST, Guid_InitTwice) {
  GuidGener::Ins().Init(1);
  Guid g1 = GuidGener::Ins().GetGuid(0);
  ASSERT_EQ(g1.mac, 1u);

  GuidGener::Ins().Init(2);
  Guid g2 = GuidGener::Ins().GetGuid(0);
  ASSERT_EQ(g2.mac, 2u);

  // 非法 mac_id 应抛
  EXPECT_THROW(GuidGener::Ins().Init(GUID_MAC_NUM), std::invalid_argument);
}

class TestObj {
 public:
  TestObj() {
    id = gid++;
    DBG_PRINT("[%llu]create obj %d", ytlib::GetThreadId(), id);
  }
  TestObj(const TestObj& obj) : data(obj.data) {
    id = gid++;
    DBG_PRINT("[%llu]create obj %d from %d by copy", ytlib::GetThreadId(), id, obj.id);
  }
  TestObj& operator=(const TestObj& obj) {
    DBG_PRINT("[%llu]copy obj %d to %d", ytlib::GetThreadId(), obj.id, id);
    data = obj.data;
    return *this;
  }
  TestObj(TestObj&& obj) : data(std::move(obj.data)) {
    id = gid++;
    DBG_PRINT("[%llu]create obj %d from %d by move", ytlib::GetThreadId(), id, obj.id);
  }
  TestObj& operator=(TestObj&& obj) {
    DBG_PRINT("[%llu]move obj %d to %d", ytlib::GetThreadId(), obj.id, id);
    data = std::move(obj.data);
    return *this;
  }
  ~TestObj() {
    DBG_PRINT("[%llu]del obj %d", ytlib::GetThreadId(), id);
  }
  uint32_t id;
  std::string data;
  static uint32_t gid;
};

uint32_t TestObj::gid = 0;

// 测试Channel
TEST(THREAD_TEST, Channel_BASE) {
  using TestChannel = Channel<TestObj>;

  std::atomic<uint32_t> ct = 0;

  auto f = [&](TestObj&& obj) {
    DBG_PRINT("[%llu]handle obj %u", ytlib::GetThreadId(), obj.id);
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

// 测试 Channel 多线程消费时所有入队元素都能被处理
TEST(THREAD_TEST, Channel_DrainOnStop) {
  using TestChannel = Channel<uint32_t>;
  std::atomic<uint32_t> ct = 0;
  TestChannel ch;
  ch.Init([&](uint32_t&& v) { ct += v; }, 4);
  ch.StartProcess();

  const uint32_t n = 1000;
  uint64_t expected = 0;
  for (uint32_t ii = 0; ii < n; ++ii) {
    ch.Enqueue(ii);
    expected += ii;
  }
  ch.StopProcess();
  ASSERT_EQ(ch.Count(), 0u);
  ASSERT_EQ(ct.load(), expected);
}

// 测试 Channel Init 重复调用应抛
TEST(THREAD_TEST, Channel_InitTwice) {
  Channel<int> ch;
  ch.Init([](int&&) {});
  ch.StartProcess();
  EXPECT_THROW(ch.Init([](int&&) {}), std::logic_error);
  ch.StopProcess();
}

// StartProcess 前没有 Init 应抛
TEST(THREAD_TEST, Channel_StartWithoutInit) {
  Channel<int> ch;
  EXPECT_THROW(ch.StartProcess(), std::logic_error);
}

// 测试BlockQueue基础同步操作
TEST(THREAD_TEST, BlockQueue_BASE) {
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
TEST(THREAD_TEST, BlockQueue_ANYSC) {
  using BckQueue = BlockQueue<TestObj>;
  TestObj::gid = 0;
  BckQueue qu(100);

  std::atomic<uint32_t> ct = 0;

  auto f = [&](TestObj&& obj) {
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

// 多生产者多消费者下不丢元素
TEST(THREAD_TEST, BlockQueue_MPMC) {
  BlockQueue<uint32_t> qu;
  std::atomic<uint64_t> sum_consumed = 0;
  std::atomic<uint32_t> consumed = 0;

  constexpr uint32_t producer_num = 4;
  constexpr uint32_t consumer_num = 4;
  constexpr uint32_t per_producer = 5000;
  constexpr uint32_t total = producer_num * per_producer;

  std::vector<std::thread> consumers;
  for (uint32_t ii = 0; ii < consumer_num; ++ii) {
    consumers.emplace_back([&] {
      uint32_t v;
      while (qu.BlockDequeue(v)) {
        sum_consumed += v;
        ++consumed;
      }
    });
  }

  std::vector<std::thread> producers;
  uint64_t expected_sum = 0;
  for (uint32_t pi = 0; pi < producer_num; ++pi) {
    for (uint32_t ii = 0; ii < per_producer; ++ii) {
      expected_sum += pi * per_producer + ii;
    }
    producers.emplace_back([&, pi] {
      for (uint32_t ii = 0; ii < per_producer; ++ii) {
        qu.Enqueue(pi * per_producer + ii);
      }
    });
  }

  for (auto& t : producers) t.join();

  // 等到队列被消费完再 Stop，避免 Stop 提前触发 BlockDequeue 返回 false 而漏元素
  while (qu.Count() > 0) std::this_thread::sleep_for(std::chrono::milliseconds(1));
  qu.Stop();

  for (auto& t : consumers) t.join();

  EXPECT_EQ(consumed.load(), total);
  EXPECT_EQ(sum_consumed.load(), expected_sum);
}

// 测试LightSignal
TEST(THREAD_TEST, LightSignal_BASE) {
  LightSignal s;
  uint32_t i = 0;

  std::thread t1([&] {
    ASSERT_EQ(i, 0);
    s.wait();  // node1
    ASSERT_EQ(i, 1);
  });

  std::thread t2([&] {
    ASSERT_EQ(i, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    i = 1;
    s.notify();  // node1
  });

  t1.join();
  t2.join();
}

// 测试LightSignal的wait_for超时与提前唤醒
TEST(THREAD_TEST, LightSignal_WaitFor) {
  LightSignal s;
  ASSERT_EQ(s.wait_for(20), false);  // 超时

  s.notify();
  ASSERT_EQ(s.wait_for(20), true);  // 已 notify

  s.reset();
  std::thread t([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    s.notify();
  });
  ASSERT_EQ(s.wait_for(1000), true);  // 在超时前被唤醒
  t.join();
}

// 测试LightSignalAtomic
TEST(THREAD_TEST, LightSignalAtomic_BASE) {
  LightSignalAtomic s;
  uint32_t i = 0;

  std::thread t1([&] {
    ASSERT_EQ(i, 0);
    s.wait();  // node1
    ASSERT_EQ(i, 1);
  });

  std::thread t2([&] {
    ASSERT_EQ(i, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    i = 1;
    s.notify();  // node1
  });

  t1.join();
  t2.join();
}

// 测试LightSignalAtomic 的 reset
TEST(THREAD_TEST, LightSignalAtomic_Reset) {
  LightSignalAtomic s;
  s.notify();
  s.wait();  // 已 notify，立刻返回
  s.reset();

  std::atomic_bool finished = false;
  std::thread t([&] {
    s.wait();
    finished = true;
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  ASSERT_FALSE(finished.load());  // reset 后 wait 应阻塞
  s.notify();
  t.join();
  ASSERT_TRUE(finished.load());
}

// 测试ThreadIdTool
TEST(THREAD_TEST, ThreadIdTool_BASE) {
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
  for (auto& itr : thread_id_map) {
    ASSERT_EQ(thread_id_set.find(itr.second) == thread_id_set.end(), true);
    thread_id_set.insert(itr.second);
  }
}

// 模拟异步请求。TODO：在win下release版本大概率会出现bug，待查验
void AsyncSendRecv(const TestObj& in_buf, std::function<void(TestObj&&)>&& callback) {
  std::thread t([&in_buf, callback{std::move(callback)}]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    TestObj out_buf;
    out_buf.data = in_buf.data + "-echo";
    callback(std::move(out_buf));
  });
  t.detach();
}

TEST(THREAD_TEST, coroutine_BASE) {
  TestObj::gid = 0;

  TestObj buf;
  buf.data = "abcd";

  auto task_fun = [&buf]() -> CoroSched<TestObj> {
    // 调用co_await后，当前协程去执行Awaitable<TestObj>的await_suspend函数
    // await_suspend函数需要确保h.resume()在之后某个时间被调用，此时返回Awaitable<TestObj>的await_resume函数的返回值
    TestObj ret_buf = co_await Awaitable<TestObj>([&buf](std::function<void(TestObj&&)>&& cb) {
      AsyncSendRecv(buf, std::move(cb));
    });
    co_yield ret_buf;  // (1)

    TestObj ret_buf2 = co_await Awaitable<TestObj>([&ret_buf](std::function<void(TestObj&&)>&& cb) {
      AsyncSendRecv(ret_buf, std::move(cb));
    });

    co_yield ret_buf2;  // (2)

    co_yield std::move(ret_buf);  // (3)

    co_return ret_buf2;  // (4)
  };

  // 开始运行协程
  auto sched = task_fun();

  // run
  TestObj out_buf = sched.Get();  // (1)
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo");

  // not run
  out_buf = sched.Get();  // (1)
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo");

  // run
  sched.Resume();
  out_buf = sched.Get();  // (2)
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo-echo");

  // run
  sched.Resume();
  out_buf = sched.Get();  // (3)
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo");

  // run
  sched.Resume();
  out_buf = sched.Get();  // (4)
  ASSERT_STREQ(out_buf.data.c_str(), "abcd-echo-echo");
}

}  // namespace ytlib
