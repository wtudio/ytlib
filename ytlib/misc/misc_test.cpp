#include <gtest/gtest.h>

#include <functional>
#include <iostream>
#include <string>

#include "dynamic_lib.hpp"
#include "guid.hpp"
#include "loop_tool.hpp"
#include "rule_tool.hpp"
#include "shared_buf.hpp"
#include "stl_util.hpp"
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

TEST(TIME_TEST, BASE_test) {
  std::string s = GetCurTimeStr();
  ASSERT_EQ(s.length(), 15);
}

TEST(STL_UTIL_TEST, Vec2Str_test) {
  struct TestCaseForVec2Str {
    std::string name;

    std::vector<std::string> v;

    std::string want_result;
  };
  std::vector<TestCaseForVec2Str> test_cases;

  test_cases.emplace_back(TestCaseForVec2Str{
      .name = "case 1",
      .v = {},
      .want_result = R"str(vec size = 0
)str"});
  test_cases.emplace_back(TestCaseForVec2Str{
      .name = "case 2",
      .v = {"", "v1", "v2\nv2",
            "12345678901234567890123456789012",
            "123456789012345678901234567890123",
            "v3"},
      .want_result = R"str(vec size = 6
[index=0]:<empty string>
[index=1]:v1
[index=2]:
v2
v2
[index=3]:12345678901234567890123456789012
[index=4]:
123456789012345678901234567890123
[index=5]:v3
)str"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Vec2Str(test_cases[ii].v);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STL_UTIL_TEST, Set2Str_test) {
  struct TestCaseForSet2Str {
    std::string name;

    std::set<std::string> s;

    std::string want_result;
  };
  std::vector<TestCaseForSet2Str> test_cases;

  test_cases.emplace_back(TestCaseForSet2Str{
      .name = "case 1",
      .s = {},
      .want_result = R"str(set size = 0
)str"});
  test_cases.emplace_back(TestCaseForSet2Str{
      .name = "case 2",
      .s = {"", "v1", "v2\nv2",
            "12345678901234567890123456789012",
            "123456789012345678901234567890123",
            "v3"},
      .want_result = R"str(set size = 6
[index=0]:<empty string>
[index=1]:12345678901234567890123456789012
[index=2]:
123456789012345678901234567890123
[index=3]:v1
[index=4]:
v2
v2
[index=5]:v3
)str"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Set2Str(test_cases[ii].s);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STL_UTIL_TEST, Map2Str_test) {
  struct TestCaseForMap2Str {
    std::string name;

    std::map<std::string, std::string> m;

    std::string want_result;
  };
  std::vector<TestCaseForMap2Str> test_cases;

  test_cases.emplace_back(TestCaseForMap2Str{
      .name = "case 1",
      .m = {},
      .want_result = R"str(map size = 0
)str"});
  test_cases.emplace_back(TestCaseForMap2Str{
      .name = "case 2",
      .m = {{"k1", "v1"}, {"k2", ""}, {"", "v3"}, {"k4\nk4", "v4"}, {"k5", "v5\nv5"}},
      .want_result = R"str(map size = 5
[index=0]:
  [key]:<empty string>
  [val]:v3
[index=1]:
  [key]:k1
  [val]:v1
[index=2]:
  [key]:k2
  [val]:<empty string>
[index=3]:
  [key]:
k4
k4
  [val]:v4
[index=4]:
  [key]:k5
  [val]:
v5
v5
)str"});
  test_cases.emplace_back(TestCaseForMap2Str{
      .name = "case 3",
      .m = {{"123456789012345678901234567890123", "v1"}, {"k2", "123456789012345678901234567890123"}},
      .want_result = R"str(map size = 2
[index=0]:
  [key]:
123456789012345678901234567890123
  [val]:v1
[index=1]:
  [key]:k2
  [val]:
123456789012345678901234567890123
)str"});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Map2Str(test_cases[ii].m);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STL_UTIL_TEST, CheckVectorEqual_test) {
  struct TestCaseForCheckVectorEqual {
    std::string name;

    std::vector<std::string> vec1;
    std::vector<std::string> vec2;

    bool want_result;
  };
  std::vector<TestCaseForCheckVectorEqual> test_cases;

  test_cases.emplace_back(TestCaseForCheckVectorEqual{
      .name = "case 1",
      .vec1 = {"v1", "v2"},
      .vec2 = {"v1", "v2"},
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckVectorEqual{
      .name = "case 2",
      .vec1 = {"v1", "v2"},
      .vec2 = {"v1", "v2", "v3"},
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckVectorEqual{
      .name = "case 3",
      .vec1 = {"v1", "v2"},
      .vec2 = {"v1", "v2x"},
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckVectorEqual{
      .name = "case 4",
      .vec1 = {},
      .vec2 = {},
      .want_result = true});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CheckVectorEqual(test_cases[ii].vec1, test_cases[ii].vec2);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STL_UTIL_TEST, CheckSetEqual_test) {
  struct TestCaseForCheckSetEqual {
    std::string name;

    std::set<std::string> set1;
    std::set<std::string> set2;

    bool want_result;
  };
  std::vector<TestCaseForCheckSetEqual> test_cases;

  test_cases.emplace_back(TestCaseForCheckSetEqual{
      .name = "case 1",
      .set1 = {"v1", "v2"},
      .set2 = {"v1", "v2"},
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckSetEqual{
      .name = "case 2",
      .set1 = {"v1", "v2"},
      .set2 = {"v1", "v2", "v3"},
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckSetEqual{
      .name = "case 3",
      .set1 = {"v1", "v2"},
      .set2 = {"v1", "v2x"},
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckSetEqual{
      .name = "case 4",
      .set1 = {},
      .set2 = {},
      .want_result = true});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CheckSetEqual(test_cases[ii].set1, test_cases[ii].set2);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STL_UTIL_TEST, CheckMapEqual_test) {
  struct TestCaseForCheckMapEqual {
    std::string name;

    std::map<std::string, std::string> map1;
    std::map<std::string, std::string> map2;

    bool want_result;
  };
  std::vector<TestCaseForCheckMapEqual> test_cases;
  test_cases.emplace_back(TestCaseForCheckMapEqual{
      .name = "case 1",
      .map1 = {},
      .map2 = {},
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckMapEqual{
      .name = "case 2",
      .map1 = {{"k1", "v1"}, {"k2", "v2"}},
      .map2 = {{"k1", "v1"}, {"k2", "v2"}},
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckMapEqual{
      .name = "case 3",
      .map1 = {{"k1", "v1"}, {"k2", "v2"}},
      .map2 = {{"k1", "v1"}, {"k2", "v2x"}},
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckMapEqual{
      .name = "case 4",
      .map1 = {{"k1", "v1"}, {"k2", "v2"}},
      .map2 = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}},
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CheckMapEqual(test_cases[ii].map1, test_cases[ii].map2);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(RULE_TOOL_TEST, RunRules) {
  int rule_result = 0;
  std::string hit_rule;

  std::map<std::string, std::function<bool()> > rule_map;
  rule_map.emplace("r1", [&]() {
    rule_result = 1;
    return false;
  });
  rule_map.emplace("r2", [&]() {
    rule_result = 2;
    return true;
  });
  rule_map.emplace("r3", [&]() {
    rule_result = 3;
    return true;
  });

  hit_rule = RunRules(std::vector<std::string>{"r1", "r2", "r3"}, rule_map);
  EXPECT_STREQ(hit_rule.c_str(), "r2");
  EXPECT_EQ(rule_result, 2);
}

}  // namespace ytlib
