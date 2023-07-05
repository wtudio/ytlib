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
  AtomicPointerBuffer<TestData> test_atomic_pointer_buffer;
  constexpr uint32_t update_count = 10000;

  std::thread t1([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (uint32_t ct = 0; ct < update_count; ++ct) {
      test_atomic_pointer_buffer.Update(new TestData(ct));
      std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
  });

  std::thread t2([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

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

}  // namespace ytlib
