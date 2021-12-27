#include <gtest/gtest.h>

#include "shared_buf.hpp"

namespace ytlib {

TEST(SHARED_BUF_TEST, BASE_test) {
  std::string s = "test test";
  uint32_t n = static_cast<uint32_t>(s.size());
  SharedBuf buf1(n);
  ASSERT_EQ(buf1.Size(), n);

  buf1 = SharedBuf(s);
  ASSERT_STREQ(std::string(buf1.Get(), n).c_str(), s.c_str());

  // 浅拷贝
  SharedBuf buf2(buf1.GetSharedPtr(), n);
  ASSERT_STREQ(std::string(buf2.Get(), n).c_str(), s.c_str());
  ASSERT_EQ(buf1.Get(), buf2.Get());

  // 深拷贝
  SharedBuf buf3(buf1.Get(), n);
  ASSERT_STREQ(std::string(buf3.Get(), n).c_str(), s.c_str());
  ASSERT_NE(buf1.Get(), buf3.Get());

  // 深拷贝
  SharedBuf buf4 = SharedBuf::GetDeepCopy(buf1);
  ASSERT_STREQ(std::string(buf4.Get(), n).c_str(), s.c_str());
  ASSERT_NE(buf1.Get(), buf4.Get());
}

}  // namespace ytlib
