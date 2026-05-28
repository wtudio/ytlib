#include <gtest/gtest.h>

#include "ring_buf.hpp"

// todo 未完成
using namespace ytlib;

TEST(RING_BUF_TEST, BASE_TEST) {
  class TestClass {
   public:
    TestClass() = default;
    TestClass(uint32_t id) : id_(id) {}

    uint32_t id_ = 0;
  };

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

TEST(RING_BUF_TEST, ARRAY_TEST) {
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

TEST(RING_BUF_TEST, ARRAY_NON_CHAR_TEST) {
  // 使用 sizeof(T) > 1 的类型，触发 PushArray/TopArray/GetArray 中
  // memcpy 字节数计算的运算符优先级缺陷（修复前 len - tmp_size * sizeof(T)
  // 会被解析为 len - (tmp_size * sizeof(T))，造成长度错算）。
  const uint32_t kBufSize = 15;
  using RingU32Buf = RingBuf<uint32_t, kBufSize>;
  RingU32Buf ring;

  std::vector<uint32_t> data;
  for (uint32_t ii = 0; ii < 10; ++ii) data.push_back(100 + ii);

  // 先填一部分再 pop 一部分，让 wpos_ 推进到尾部，制造下一次 PushArray 跨界
  ASSERT_EQ(ring.PushArray(data.data(), 8), true);
  ASSERT_EQ(ring.PopArray(8), true);
  // 此时 rpos_=wpos_=8，下一次 PushArray(10) 必跨 BUF_SIZE 边界

  ASSERT_EQ(ring.PushArray(data.data(), 10), true);
  ASSERT_EQ(ring.Size(), 10u);

  // 从环绕状态读取，要求 TopArray 在读区间未跨边界时返回指向内部存储的指针
  uint32_t* p = nullptr;
  // 读前 7 个：cur_rpos=8, len=7, 8+7=15<=15 不跨界，应直接返回内部指针
  ASSERT_EQ(ring.GetArray(0, p, 7), true);
  ASSERT_NE(p, nullptr);
  for (uint32_t ii = 0; ii < 7; ++ii) {
    EXPECT_EQ(p[ii], data[ii]);
  }

  // 读全部 10 个：必须跨界，调用方需提供缓冲区
  uint32_t out_buf[10] = {0};
  uint32_t* out_ptr = out_buf;
  ASSERT_EQ(ring.GetArray(0, out_ptr, 10), true);
  for (uint32_t ii = 0; ii < 10; ++ii) {
    EXPECT_EQ(out_buf[ii], data[ii])
        << "mismatch at index " << ii << ", got " << out_buf[ii];
  }

  // TopArray 同样验证
  uint32_t out_buf2[10] = {0};
  uint32_t* top_ptr = out_buf2;
  ASSERT_EQ(ring.TopArray(top_ptr, 10), true);
  for (uint32_t ii = 0; ii < 10; ++ii) {
    EXPECT_EQ(out_buf2[ii], data[ii])
        << "TopArray mismatch at index " << ii;
  }

  ASSERT_EQ(ring.PopArray(10), true);
  EXPECT_EQ(ring.Size(), 0u);
}

TEST(RING_BUF_TEST, WRAP_CONTIGUOUS_READ_TEST) {
  // 修复前 TopArray/GetArray 用 rpos_+len<=wpos_ 判定数据连续，
  // 在 wpos_<rpos_ 的环绕态下即使读区间未跨 BUF_SIZE 也会错判为跨界，
  // 此时若调用方未提供 buf（buf==nullptr），原实现会返回 false。
  const uint32_t kBufSize = 10;
  using RingCharBuf = RingBuf<char, kBufSize>;
  RingCharBuf ring;

  // 推进到 wpos_=8, rpos_=8 的状态
  std::string filler = "01234567";
  ASSERT_EQ(ring.PushArray(filler.c_str(), 8), true);
  ASSERT_EQ(ring.PopArray(8), true);

  // 写入 5 个字节：wpos_ 跨界到 3，形成 wpos_=3 < rpos_=8 的环绕态
  std::string s = "ABCDE";
  ASSERT_EQ(ring.PushArray(s.c_str(), 5), true);
  ASSERT_EQ(ring.Size(), 5u);

  // 读区间 [rpos_=8, 8+2)=[8,10) 不跨 BUF_SIZE，应返回内部指针，无需提供 buf
  char* ps = nullptr;
  ASSERT_EQ(ring.TopArray(ps, 2), true);
  ASSERT_NE(ps, nullptr);
  EXPECT_EQ(ps[0], 'A');
  EXPECT_EQ(ps[1], 'B');

  // GetArray 同理：从 pos=0 起读 2 个，区间在尾部连续段内
  ps = nullptr;
  ASSERT_EQ(ring.GetArray(0, ps, 2), true);
  ASSERT_NE(ps, nullptr);
  EXPECT_EQ(ps[0], 'A');
  EXPECT_EQ(ps[1], 'B');

  // 整体 5 字节跨界，需要外部 buf
  char out[5] = {0};
  ps = out;
  ASSERT_EQ(ring.TopArray(ps, 5), true);
  EXPECT_EQ(std::string(out, 5), s);
}
