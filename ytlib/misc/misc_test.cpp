#include <gtest/gtest.h>

#include <functional>
#include <iostream>
#include <string>

#include "dynamic_lib.hpp"
#include "guid.hpp"
#include "loop_tool.hpp"
#include "print_helper.hpp"

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
    for (uint32_t ii = lt.m_vecContent.size() - 1; ii > 0; --ii) {
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

struct TestObj {
  int age;
  std::string name;

  friend std::ostream& operator<<(std::ostream& out, const TestObj& obj) {
    out << "age:" << obj.age << "\n";
    out << "name:" << obj.name << "\n";
    return out;
  }
};

TEST(MISC_TEST, PrintHelper_BASE) {
  PrintHelper::Ins().SetPrint(true);
  ASSERT_TRUE(PrintHelper::Ins().IfPrint());

  TestObj obj = {20, "testname"};
  std::string struct_str = R"str(test msg:
age:20
name:testname
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintStruct("test msg", obj).c_str(), struct_str.c_str());

  std::vector<std::string> v = {"val1", "val2", "val3\nval3val3"};
  std::string v_str = R"str(test vec:
vec size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintVec("test vec", v).c_str(), v_str.c_str());

  std::set<std::string> s = {"val1", "val2", "val3\nval3val3"};
  std::string s_str = R"str(test set:
set size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintSet("test set", s).c_str(), s_str.c_str());

  std::map<std::string, std::string> m = {{"key1", "val1"}, {"key2", "val2"}, {"key3", "val3\nval3val3"}};
  std::string m_str = R"str(test map:
map size = 3
[index=0, key=key1]:val1
[index=1, key=key2]:val2
[index=2, key=key3]:
val3
val3val3
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintMap("test map", m).c_str(), m_str.c_str());
}

}  // namespace ytlib
