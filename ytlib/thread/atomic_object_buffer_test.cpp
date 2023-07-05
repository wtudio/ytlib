#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "atomic_object_buffer.hpp"

namespace ytlib {

std::atomic_uint32_t construction_count = 0;
std::atomic_uint32_t deconstruction_count = 0;

class TestData {
 public:
  TestData(uint32_t n)
      : s("count " + std::to_string(n)),
        count(n) {
    ++construction_count;
  }
  ~TestData() {
    ++deconstruction_count;
  }

  std::string s;
  uint32_t count;
};

TEST(THREAD_TEST, AtomicObjectBuffer) {
  AtomicObjectBuffer<TestData> test_atomic_object_buffer;
  constexpr uint32_t update_count = 10000;
  uint32_t take_count = 0;
  uint32_t update_take_count = 0;

  std::thread t1([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (uint32_t ct = 0; ct < update_count; ++ct) {
      // test_atomic_object_buffer.Update(new TestData(ct));
      TestData* p = test_atomic_object_buffer.TakeAndUpdate(new TestData(ct));
      if (p != nullptr) {
        delete p;
        ++update_take_count;
      }
      std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
  });

  std::thread t2([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (uint32_t ct = 0; ct < update_count; ++ct) {
      TestData* p = test_atomic_object_buffer.Take();
      if (p != nullptr) {
        delete p;
        ++take_count;
      }
      std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
  });

  t1.join();
  t2.join();

  TestData* p = test_atomic_object_buffer.TakeAndUpdate(nullptr);
  if (p != nullptr) {
    delete p;
    ++take_count;
  }

  EXPECT_EQ(update_count, take_count + update_take_count);
  EXPECT_EQ(construction_count.load(), update_count);
  EXPECT_EQ(construction_count.load(), deconstruction_count.load());
  EXPECT_LE(take_count, update_count);
}

}  // namespace ytlib
