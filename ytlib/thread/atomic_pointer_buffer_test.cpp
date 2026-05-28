#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "atomic_pointer_buffer.hpp"

namespace ytlib {

std::atomic_uint32_t construction_count = 0;
std::atomic_uint32_t deconstruction_count = 0;

class TestData {
 public:
  TestData(uint32_t n) : s("count " + std::to_string(n)), count(n) { ++construction_count; }
  ~TestData() { ++deconstruction_count; }

  std::string s;
  uint32_t count;
};

TEST(THREAD_TEST, AtomicPointerBuffer) {
  construction_count = 0;
  deconstruction_count = 0;

  AtomicPointerBuffer<TestData> test_atomic_pointer_buffer;
  constexpr uint32_t update_count = 3000;

  std::thread t1([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    for (uint32_t ct = 0; ct < update_count; ++ct) {
      test_atomic_pointer_buffer.Update(new TestData(ct));
      std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
  });

  std::thread t2([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    for (uint32_t ct = 0; ct < update_count; ++ct) {
      TestData* p = test_atomic_pointer_buffer.Take();
      if (p != nullptr) delete p;
      std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
  });

  t1.join();
  t2.join();

  TestData* p = test_atomic_pointer_buffer.TakeAndUpdate(nullptr);
  if (p != nullptr) delete p;

  EXPECT_EQ(construction_count.load(), update_count);
  EXPECT_EQ(construction_count.load(), deconstruction_count.load());
}

// 单线程基本路径
TEST(THREAD_TEST, AtomicPointerBuffer_SingleThread) {
  construction_count = 0;
  deconstruction_count = 0;

  {
    AtomicPointerBuffer<TestData> buf;

    // Take 空指针返回 nullptr
    EXPECT_EQ(buf.Take(), nullptr);

    // Update：原指针被 buf 释放
    buf.Update(new TestData(1));
    EXPECT_EQ(construction_count.load(), 1u);
    buf.Update(new TestData(2));
    EXPECT_EQ(construction_count.load(), 2u);
    EXPECT_EQ(deconstruction_count.load(), 1u);

    // TakeAndUpdate：原指针交还调用方
    TestData* p = buf.TakeAndUpdate(new TestData(3));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->count, 2u);
    delete p;
    EXPECT_EQ(deconstruction_count.load(), 2u);

    // Take：清空缓存
    p = buf.Take();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->count, 3u);
    delete p;
    EXPECT_EQ(buf.Take(), nullptr);
  }

  // 析构时应释放剩余指针。这里 Take 已置空，析构 noop
  EXPECT_EQ(construction_count.load(), deconstruction_count.load());

  // 析构时若仍有指针，buf 负责释放
  construction_count = 0;
  deconstruction_count = 0;
  {
    AtomicPointerBuffer<TestData> buf;
    buf.Update(new TestData(42));
  }
  EXPECT_EQ(construction_count.load(), 1u);
  EXPECT_EQ(deconstruction_count.load(), 1u);
}

}  // namespace ytlib
