#include <gtest/gtest.h>

#include "ref_counter.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

class RefCounterObj {
 public:
  RefCounterObj() {
    DBG_PRINT("create obj");
  }

  RefCounterObj(const RefCounterObj &obj) = delete;
  RefCounterObj &operator=(const RefCounterObj &obj) = delete;
  RefCounterObj(RefCounterObj &&obj) = delete;
  RefCounterObj &operator=(RefCounterObj &&obj) = delete;

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

}  // namespace ytlib