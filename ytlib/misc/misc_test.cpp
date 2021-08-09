#include <gtest/gtest.h>

#include <functional>
#include <iostream>
#include <string>

#include "dynamic_lib.hpp"
#include "guid.hpp"
#include "loop_tool.hpp"
#include "shared_buf.hpp"
#include "test_util.hpp"
#include "time.hpp"

namespace ytlib {

using std::cout;
using std::endl;
using std::vector;

TEST(MISC_TEST, GUID_BASE) {
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
    // printf("%llu\n", guid_cur.id);
    ASSERT_GE(guid_cur.id, guid_last.id);
    guid_last = guid_cur;
  }
}

TEST(MISC_TEST, LoopTool_BASE) {
  vector<uint32_t> vec{2, 3, 3};
  LoopTool lt(vec);
  do {
    for (size_t ii = lt.m_vecContent.size() - 1; ii > 0; --ii) {
      cout << lt.m_vecContent[ii] << '-';
    }
    cout << lt.m_vecContent[0] << endl;

  } while (++lt);
}

TEST(MISC_TEST, DynamicLib_BASE) {
  DynamicLib d;
  ASSERT_FALSE(d);
  ASSERT_STREQ(d.GetLibName().c_str(), "");
  ASSERT_FALSE(d.Load("xxx/xxx"));

  ASSERT_FALSE(DynamicLibContainer::Ins().LoadLib("xxx/xxx"));
  ASSERT_FALSE(DynamicLibContainer::Ins().GetLib("xxx/xxx"));
  ASSERT_FALSE(DynamicLibContainer::Ins().RemoveLib("xxx/xxx"));
}

TEST(MISC_TEST, sharedBuf_BASE) {
  std::string s = "test test";
  size_t n = s.size();
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

TEST(MISC_TEST, TIME_BASE) {
  std::string s = GetCurTimeStr();
  ASSERT_EQ(s.length(), 15);
}

TEST(TEST_UTIL_TEST, BASE) {
  std::vector<std::string> v1{"aaa", "bbb", "ccc"};
  std::vector<std::string> v2{"aaa", "bbb", "ccc"};
  std::vector<std::string> v3{"aaa", "bbb", "cccc"};

  EXPECT_TRUE(CheckVectorEqual(v1, v2));
  EXPECT_FALSE(CheckVectorEqual(v1, v3));

  std::set<std::string> set1{"aaa", "bbb", "ccc"};
  std::set<std::string> set2{"aaa", "bbb", "ccc"};
  std::set<std::string> set3{"aaa", "bbb", "cccc"};

  EXPECT_TRUE(CheckSetEqual(set1, set2));
  EXPECT_FALSE(CheckSetEqual(set1, set3));

  std::map<std::string, std::string> map1{{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}};
  std::map<std::string, std::string> map2{{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}};
  std::map<std::string, std::string> map3{{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3x"}};

  EXPECT_TRUE(CheckMapEqual(map1, map2));
  EXPECT_FALSE(CheckMapEqual(map1, map3));
}

}  // namespace ytlib
