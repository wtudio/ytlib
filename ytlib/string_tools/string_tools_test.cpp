#include <gtest/gtest.h>

#include "string_algs.hpp"
#include "string_tools.hpp"
#include "url_encode.hpp"

using std::pair;
using std::string;
using std::vector;

namespace ytlib {

TEST(STRING_TEST, ALG_TEST) {
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

TEST(STRING_TEST, trim_TEST) {
  string str = " testval  ";
  EXPECT_STREQ(trim(str).c_str(), "testval");
  EXPECT_STREQ(str.c_str(), "testval");
  EXPECT_STREQ(trim(str).c_str(), "testval");
}

TEST(STRING_TEST, Replace_TEST) {
  string str = "val1+val2+val3";
  EXPECT_STREQ(ReplaceString(str, "val1", "val10").c_str(), "val10+val2+val3");
  EXPECT_STREQ(ReplaceString(str, "val2", "v2").c_str(), "val10+v2+val3");
  EXPECT_STREQ(ReplaceString(str, "val3", "val4").c_str(), "val10+v2+val4");
}

TEST(STRING_TEST, IsAlnumStr_TEST) {
  EXPECT_EQ(IsAlnumStr("123456789"), true);
  EXPECT_EQ(IsAlnumStr("123456789abcd"), true);
  EXPECT_EQ(IsAlnumStr("123456789.."), false);
}

TEST(STRING_TEST, Map_TEST) {
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

TEST(STRING_TEST, Vector_TEST) {
  string str = "v1,v2,, v3 , ,v4,";
  auto revec = SplitToVec(str, ",");
  EXPECT_EQ(revec.size(), 4);
  EXPECT_STREQ(revec[0].c_str(), "v1");
  EXPECT_STREQ(revec[1].c_str(), "v2");
  EXPECT_STREQ(revec[2].c_str(), "v3");
  EXPECT_STREQ(revec[3].c_str(), "v4");

  EXPECT_STREQ(JoinVec(revec, "|").c_str(), "v1|v2|v3|v4");

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

TEST(STRING_TEST, Set_TEST) {
  std::set<std::string> st{"v1", "v2", "v3", "v4"};
  EXPECT_STREQ(JoinSet(st, "|").c_str(), "v1|v2|v3|v4");
}

TEST(STRING_TEST, Version_TEST) {
  EXPECT_EQ(CmpVersion("7.6.8.11020", "7.6.8"), 1);
  EXPECT_EQ(CmpVersion("7.6.8", "7.6.8"), 0);
  EXPECT_EQ(CmpVersion("7.6.8", "7.6.8.11020"), -1);
  EXPECT_EQ(CmpVersion("10.0.1.11020", "7.6.8"), 1);
  EXPECT_EQ(CmpVersion("9.8.0", "9..8.0"), 0);

  EXPECT_EQ(CheckVersionInside("7.6.8", "7.6.7", "7.6.9"), true);
  EXPECT_EQ(CheckVersionInside("7.6.8", "7.6.8", "7.6.8"), true);
  EXPECT_EQ(CheckVersionInside("7.6.6", "7.6.7", "7.6.9"), false);
}

TEST(STRING_TEST, UrlEncode_TEST) {
  string s = "http://abc123.com/aaa/bbbb?qa=1&qb=adf";

  string se = UrlEncode(s, false);
  EXPECT_STREQ(se.c_str(), "http%3a%2f%2fabc123.com%2faaa%2fbbbb%3fqa%3d1%26qb%3dadf");

  string sd = UrlDecode(se);
  EXPECT_STREQ(sd.c_str(), s.c_str());
}
}  // namespace ytlib
