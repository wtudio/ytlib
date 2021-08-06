#include <gtest/gtest.h>

#include "string_algs.hpp"
#include "string_util.hpp"
#include "tstring.hpp"
#include "url_encode.hpp"

using std::pair;
using std::string;
using std::vector;

namespace ytlib {

TEST(STRING_TEST, ALG_BASE) {
  //kmp
  string ss = "abcdef abcdefg abcdefgh";
  ASSERT_EQ(KMP(ss, "abcdef"), 0);
  ASSERT_EQ(KMP(ss, "abcdefg"), 7);
  ASSERT_EQ(KMP(ss, "abcdefgh"), 15);
  ASSERT_EQ(KMP(ss, "aaaa"), ss.length());

  //StrDif
  ASSERT_EQ(StrDif("abcdxfg", "abcdefg"), 1);
  ASSERT_EQ(StrDif("abcdfg", "abcdefg"), 1);
  ASSERT_EQ(StrDif("abcdfg", "abcdef"), 2);

  //LongestSubStrWithoutDup
  pair<std::size_t, std::size_t> re = LongestSubStrWithoutDup("arabcacfr");
  ASSERT_EQ(re.first, 1);
  ASSERT_EQ(re.second, 4);
}

TEST(STRING_TEST, trim_BASE) {
  string str = " testval  ";
  EXPECT_STREQ(trim(str).c_str(), "testval");
  EXPECT_STREQ(str.c_str(), "testval");
  EXPECT_STREQ(trim(str).c_str(), "testval");
}

TEST(STRING_TEST, Replace_BASE) {
  string str = "val1+val2+val3";
  EXPECT_STREQ(ReplaceString(str, "val1", "val10").c_str(), "val10+val2+val3");
  EXPECT_STREQ(ReplaceString(str, "val2", "v2").c_str(), "val10+v2+val3");
  EXPECT_STREQ(ReplaceString(str, "val3", "val4").c_str(), "val10+v2+val4");
}

TEST(STRING_TEST, IsAlnumStr_BASE) {
  EXPECT_EQ(IsAlnumStr("123456789"), true);
  EXPECT_EQ(IsAlnumStr("123456789abcd"), true);
  EXPECT_EQ(IsAlnumStr("123456789.."), false);
}

TEST(STRING_TEST, Map_BASE) {
  string str = "k1=v1& k2 =v2&k3= v3 &&k4=v4&k4=v4x& =v5&k6= &";
  auto remap = SplitToMap(str);
  EXPECT_STREQ(GetMapItemWithDef(remap, "k1").c_str(), "v1");
  EXPECT_STREQ(GetMapItemWithDef(remap, "k2").c_str(), "v2");
  EXPECT_STREQ(GetMapItemWithDef(remap, "k3").c_str(), "v3");
  EXPECT_STREQ(GetMapItemWithDef(remap, "k4").c_str(), "v4x");
  EXPECT_STREQ(GetMapItemWithDef(remap, "").c_str(), "v5");
  EXPECT_STREQ(GetMapItemWithDef(remap, "k6").c_str(), "");
  EXPECT_STREQ(GetMapItemWithDef(remap, "k7", "default").c_str(), "default");

  string str1 = JoinMap(remap);
  EXPECT_STREQ(str1.c_str(), "=v5&k1=v1&k2=v2&k3=v3&k4=v4x&k6=");

  AddKV(str1, "k8", "v8");
  EXPECT_STREQ(str1.c_str(), "=v5&k1=v1&k2=v2&k3=v3&k4=v4x&k6=&k8=v8");
  string str2 = "";
  AddKV(str2, "k8", "v8");
  EXPECT_STREQ(str2.c_str(), "k8=v8");

  string str3 = "k10=v10& k11 = v11 &k12 = v12 ";
  EXPECT_STREQ(GetValueFromStrKV(str3, "k10").c_str(), "v10");
  EXPECT_STREQ(GetValueFromStrKV(str3, "k11").c_str(), "v11");
  EXPECT_STREQ(GetValueFromStrKV(str3, "k12").c_str(), "v12");
  EXPECT_STREQ(GetValueFromStrKV(str3, "k12", "&", "=", false).c_str(), " v12 ");

  std::map<std::string, std::map<std::string, std::string> > test_map;
  test_map["111"].clear();
  test_map["222"].clear();
  test_map["333"].clear();
  std::set<std::string> test_set = GetMapKeys(test_map);
  EXPECT_EQ(test_set.size(), 3);
}

TEST(STRING_TEST, Vector_BASE) {
  struct TestCaseForSplitToVec {
    std::string name;

    std::string source;
    std::string sep;
    bool trimempty;

    std::vector<std::string> want_result;
  };
  std::vector<TestCaseForSplitToVec> test_cases;

  test_cases.emplace_back(TestCaseForSplitToVec{
      "case 1",
      "v1,v2, v3,v4",
      ",",
      true,
      {"v1", "v2", "v3", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      "case 2",
      "v1,v2, v3,v4",
      ",",
      false,
      {"v1", "v2", " v3", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      "case 3",
      "v1,v2,, v3 , ,v4,",
      ",",
      true,
      {"v1", "v2", "v3", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      "case 4",
      "v1,v2,, v3 , ,v4,",
      ",",
      false,
      {"v1", "v2", " v3 ", " ", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      "bad case 1",
      " ",
      ",",
      true,
      {}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      "bad case 2",
      " ",
      ",",
      false,
      {" "}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      "bad case 3",
      " , ",
      ",",
      false,
      {" ", " "}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = SplitToVec(
        test_cases[ii].source,
        test_cases[ii].sep,
        test_cases[ii].trimempty);
    ASSERT_EQ(ret.size(), test_cases[ii].want_result.size())
        << "SplitToVec Test " << test_cases[ii].name << " failed";
    for (size_t jj = 0; jj < ret.size(); ++jj) {
      EXPECT_STREQ(ret[jj].c_str(), test_cases[ii].want_result[jj].c_str())
          << "SplitToVec Test " << test_cases[ii].name << " failed";
    }
  }

  std::vector<std::string> vec{"v1", "v2", "v3", "v4"};
  EXPECT_STREQ(JoinVec(vec, "|").c_str(), "v1|v2|v3|v4");

  EXPECT_EQ(CheckIfInList("123456", "123456"), true);
  EXPECT_EQ(CheckIfInList("0123456", "123456"), false);
  EXPECT_EQ(CheckIfInList("1234567", "123456"), false);
  EXPECT_EQ(CheckIfInList("123456,", "123456"), true);
  EXPECT_EQ(CheckIfInList(",123456", "123456"), true);
  EXPECT_EQ(CheckIfInList("aaa,123456,bbb", "123456"), true);
  EXPECT_EQ(CheckIfInList("aaa,0123456,bbb", "123456"), false);
  EXPECT_EQ(CheckIfInList("aaa,1234567,bbb", "123456"), false);
  EXPECT_EQ(CheckIfInList("aaa,1234567,123456,bbb", "123456"), true);
  EXPECT_EQ(CheckIfInList("123,456", "123,456", ','), false);
  EXPECT_EQ(CheckIfInList("aaa,,bbb", "", ','), false);
}

TEST(STRING_TEST, Set_BASE) {
  std::set<std::string> st{"v1", "v2", "v3", "v4"};
  EXPECT_STREQ(JoinSet(st, "|").c_str(), "v1|v2|v3|v4");
}

TEST(STRING_TEST, Version_BASE) {
  EXPECT_EQ(CmpVersion("7.6.8.11020", "7.6.8"), 1);
  EXPECT_EQ(CmpVersion("7.6.8", "7.6.8"), 0);
  EXPECT_EQ(CmpVersion("7.6.8", "7.6.8.11020"), -1);
  EXPECT_EQ(CmpVersion("10.0.1.11020", "7.6.8"), 1);
  EXPECT_EQ(CmpVersion("9.8.0", "9..8.0"), 0);

  EXPECT_EQ(CheckVersionInside("7.6.8", "7.6.7", "7.6.9"), true);
  EXPECT_EQ(CheckVersionInside("7.6.8", "7.6.8", "7.6.8"), true);
  EXPECT_EQ(CheckVersionInside("7.6.6", "7.6.7", "7.6.9"), false);
}

TEST(STRING_TEST, UrlEncode_BASE) {
  string s = "http://abc123.com/aaa/bbbb?qa=1&qb=adf";

  string se = UrlEncode(s, false);
  EXPECT_STREQ(se.c_str(), "http%3a%2f%2fabc123.com%2faaa%2fbbbb%3fqa%3d1%26qb%3dadf");

  string sd = UrlDecode(se);
  EXPECT_STREQ(sd.c_str(), s.c_str());
}

TEST(STRING_TEST, tstring_BASE) {
  std::string s = "test123";

  std::wstring ws = ytlib::ToWString(s);
  EXPECT_STREQ(ws.c_str(), L"test123");

  std::string s1 = ytlib::ToString(ws);
  EXPECT_STREQ(s1.c_str(), "test123");
}

}  // namespace ytlib
