#include <gtest/gtest.h>

#include <functional>
#include <iostream>
#include <string>

#include "print_ctr.hpp"
#include "stl_util.hpp"
#include "time.hpp"

namespace ytlib {

using std::cout;
using std::endl;
using std::vector;

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

struct TestObj {
  int age;
  std::string name;

  friend std::ostream& operator<<(std::ostream& out, const TestObj& obj) {
    out << "age:" << obj.age << "\n";
    out << "name:" << obj.name << "\n";
    return out;
  }
};

TEST(LOG_TEST, PrintHelper_BASE) {
  PrintCtr::Ins().SetPrint(true);
  ASSERT_TRUE(PrintCtr::Ins().IfPrint());

  TestObj obj = {20, "testname"};
  std::string struct_str = R"str(test msg:
age:20
name:testname

)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintStruct("test msg", obj).c_str(), struct_str.c_str());

  std::vector<std::string> v = {"val1", "val2", "val3\nval3val3"};
  std::string v_str = R"str(test vec:
vec size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintVec("test vec", v).c_str(), v_str.c_str());

  std::set<std::string> s = {"val1", "val2", "val3\nval3val3"};
  std::string s_str = R"str(test set:
set size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintSet("test set", s).c_str(), s_str.c_str());

  std::map<std::string, std::string> m = {{"key1", "val1"}, {"key2", "val2"}, {"key3", "val3\nval3val3"}};
  std::string m_str = R"str(test map:
map size = 3
[index=0]:
  [key]:key1
  [val]:val1
[index=1]:
  [key]:key2
  [val]:val2
[index=2]:
  [key]:key3
  [val]:
val3
val3val3
)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintMap("test map", m).c_str(), m_str.c_str());
}

}  // namespace ytlib
