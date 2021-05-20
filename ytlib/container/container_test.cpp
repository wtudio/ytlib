#include <gtest/gtest.h>

#include "ring_buf.hpp"

namespace ytlib {

class TestClass {
 public:
  TestClass() {}
  TestClass(uint32_t id) : id_(id) {}

  uint32_t id_ = 0;
};

TEST(RSTUDIO_UTILS_TEST, RingBuf_TEST) {
  const uint32_t kBufSize = 10;
  using RingBufTest = RingBuf<TestClass, kBufSize>;
  RingBufTest ring;

  ASSERT_EQ(ring.Empty(), true);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Capacity(), kBufSize - 1);
  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);

  TestClass obj1(1);
  TestClass obj2(2);

  ASSERT_EQ(ring.Push(obj1), true);
  ASSERT_EQ(ring.Push(std::move(obj2)), true);

  ASSERT_EQ(ring.Empty(), false);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Size(), 2);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 3);

  ASSERT_EQ(ring.Top().id_, 1);
  ASSERT_EQ(ring.Get(0).id_, 1);
  ASSERT_EQ(ring.Get(1).id_, 2);

  for (uint32_t ii = 3; ii < kBufSize; ++ii) {
    ASSERT_EQ(ring.Push(TestClass(ii)), true);
  }

  ASSERT_EQ(ring.Empty(), false);
  ASSERT_EQ(ring.Full(), true);
  ASSERT_EQ(ring.Size(), kBufSize - 1);
  ASSERT_EQ(ring.UnusedCapacity(), 0);

  ASSERT_EQ(ring.Push(TestClass(kBufSize)), false);

  ASSERT_EQ(ring.Pop(), true);

  ASSERT_EQ(ring.Empty(), false);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Size(), kBufSize - 2);
  ASSERT_EQ(ring.UnusedCapacity(), 1);

  ring.Clear();
  ASSERT_EQ(ring.Pop(), false);

  ASSERT_EQ(ring.Empty(), true);
  ASSERT_EQ(ring.Full(), false);
  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);
}

TEST(RSTUDIO_UTILS_TEST, RingBuf_char_TEST) {
  const uint32_t kBufSize = 15;
  using RingCharBufTest = RingBuf<char, kBufSize>;
  RingCharBufTest ring;

  std::string s = "0123456789";
  uint32_t s_size = static_cast<uint32_t>(s.size());
  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), true);

  ASSERT_EQ(ring.Size(), s_size);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - s_size - 1);

  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), false);

  char* ps = nullptr;
  ASSERT_EQ(ring.TopArray(ps, s_size), true);

  ASSERT_STREQ(std::string(ps, s_size).c_str(), s.c_str());

  uint32_t get_pos = 5;
  ASSERT_EQ(ring.GetArray(get_pos, ps, s_size - get_pos), true);

  ASSERT_STREQ(std::string(ps, s_size - get_pos).c_str(), s.substr(get_pos).c_str());

  ASSERT_EQ(ring.PopArray(s_size), true);
  ASSERT_EQ(ring.PopArray(s_size), false);

  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);

  // -----------------------

  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), true);

  ASSERT_EQ(ring.Size(), s_size);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - s_size - 1);

  ASSERT_EQ(ring.PushArray(s.c_str(), s_size), false);

  ps = nullptr;
  ASSERT_EQ(ring.TopArray(ps, s_size), false);

  char ps2[kBufSize];
  ASSERT_EQ(ring.TopArray(ps = ps2, s_size), true);

  ASSERT_STREQ(std::string(ps, s_size).c_str(), s.c_str());

  ASSERT_EQ(ring.PopArray(s_size), true);
  ASSERT_EQ(ring.PopArray(s_size), false);

  ASSERT_EQ(ring.Size(), 0);
  ASSERT_EQ(ring.UnusedCapacity(), kBufSize - 1);
}

}  // namespace ytlib
