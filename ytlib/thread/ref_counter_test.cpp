#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "ref_counter.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

class RefCounterObj {
 public:
  RefCounterObj() {
    DBG_PRINT("create obj");
  }

  RefCounterObj(const RefCounterObj& obj) = delete;
  RefCounterObj& operator=(const RefCounterObj& obj) = delete;
  RefCounterObj(RefCounterObj&& obj) = delete;
  RefCounterObj& operator=(RefCounterObj&& obj) = delete;

  ~RefCounterObj() {
    DBG_PRINT("del obj");
  }
};

TEST(THREAD_TEST, RefCounter) {
  // 构造函数
  RefCounter<RefCounterObj> obj_1(new RefCounterObj());
  EXPECT_EQ(obj_1.Counter(), 1);

  // 拷贝构造函数
  RefCounter<RefCounterObj> obj_2(obj_1);
  EXPECT_EQ(obj_1.Counter(), 2);
  EXPECT_EQ(obj_2.Counter(), 2);

  EXPECT_EQ(obj_1.Get(), obj_2.Get());

  // 移动构造函数
  RefCounter<RefCounterObj> obj_3(std::move(obj_1));
  EXPECT_EQ(obj_2.Counter(), 2);
  EXPECT_EQ(obj_3.Counter(), 2);

  EXPECT_EQ(obj_2.Get(), obj_3.Get());

  // 拷贝赋值函数
  auto obj_4 = MakeRefCounter<RefCounterObj>();
  EXPECT_EQ(obj_4.Counter(), 1);

  obj_4 = obj_2;
  EXPECT_EQ(obj_2.Counter(), 3);
  EXPECT_EQ(obj_3.Counter(), 3);
  EXPECT_EQ(obj_4.Counter(), 3);

  // 移动赋值函数
  obj_4 = std::move(obj_2);
  EXPECT_EQ(obj_3.Counter(), 2);
  EXPECT_EQ(obj_4.Counter(), 2);
}

// 移动后访问保留对象应安全：Get 返回 nullptr、Counter 返回 0
TEST(THREAD_TEST, RefCounter_MovedFrom) {
  RefCounter<RefCounterObj> a(new RefCounterObj());

  RefCounter<RefCounterObj> b(std::move(a));
  EXPECT_EQ(a.Get(), nullptr);
  EXPECT_EQ(a.Counter(), 0u);
  EXPECT_NE(b.Get(), nullptr);
  EXPECT_EQ(b.Counter(), 1u);

  // 把已被 move 的空状态对象拷贝进来，应替换原 ref，原 ref 的对象会被释放
  RefCounter<RefCounterObj> c(new RefCounterObj());
  c = a;
  EXPECT_EQ(c.Get(), nullptr);
  EXPECT_EQ(c.Counter(), 0u);
}

// 多线程下并发拷贝/析构计数应一致，验证 fetch_add/fetch_sub 的内存序正确
TEST(THREAD_TEST, RefCounter_Concurrent) {
  auto obj = MakeRefCounter<RefCounterObj>();
  EXPECT_EQ(obj.Counter(), 1u);

  constexpr uint32_t thread_num = 8;
  constexpr uint32_t per_thread = 1000;

  std::vector<std::thread> threads;
  threads.reserve(thread_num);
  for (uint32_t ii = 0; ii < thread_num; ++ii) {
    threads.emplace_back([&] {
      for (uint32_t jj = 0; jj < per_thread; ++jj) {
        RefCounter<RefCounterObj> copy = obj;
        EXPECT_NE(copy.Get(), nullptr);
      }
    });
  }
  for (auto& t : threads) t.join();

  // 所有 copy 都已析构，最外层 obj 的计数应回到 1
  EXPECT_EQ(obj.Counter(), 1u);
}

}  // namespace ytlib