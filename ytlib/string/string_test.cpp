#include <gtest/gtest.h>

#include "string_algs.hpp"
#include "string_util.hpp"
#include "tstring.hpp"
#include "url_encode.hpp"
#include "ytlib/misc/test_util.hpp"

using std::pair;
using std::string;
using std::vector;

namespace ytlib {

TEST(STRING_ALGS_TEST, KMP) {
  struct TestCaseForKMP {
    std::string name;

    std::string ss;
    std::string ps;

    std::size_t want_result;
  };
  std::vector<TestCaseForKMP> test_cases;

  test_cases.emplace_back(TestCaseForKMP{
      .name = "case 1",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "abcdef",
      .want_result = 0});
  test_cases.emplace_back(TestCaseForKMP{
      .name = "case 2",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "abcdefg",
      .want_result = 7});
  test_cases.emplace_back(TestCaseForKMP{
      .name = "case 3",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "abcdefgh",
      .want_result = 15});
  test_cases.emplace_back(TestCaseForKMP{
      .name = "case 4",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "aaaa",
      .want_result = 23});
  test_cases.emplace_back(TestCaseForKMP{
      .name = "bad case 1",
      .ss = "123",
      .ps = "",
      .want_result = 3});
  test_cases.emplace_back(TestCaseForKMP{
      .name = "bad case 2",
      .ss = "123",
      .ps = "4",
      .want_result = 3});
  test_cases.emplace_back(TestCaseForKMP{
      .name = "bad case 3",
      .ss = "123",
      .ps = "1234",
      .want_result = 3});
  test_cases.emplace_back(TestCaseForKMP{
      .name = "bad case 4",
      .ss = "",
      .ps = "",
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = KMP(
        test_cases[ii].ss,
        test_cases[ii].ps);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_ALGS_TEST, StrDif) {
  struct TestCaseForStrDif {
    std::string name;

    std::string s1;
    std::string s2;

    std::size_t want_result;
  };
  std::vector<TestCaseForStrDif> test_cases;

  test_cases.emplace_back(TestCaseForStrDif{
      .name = "case 1",
      .s1 = "abcdxfg",
      .s2 = "abcdefg",
      .want_result = 1});
  test_cases.emplace_back(TestCaseForStrDif{
      .name = "case 2",
      .s1 = "abcdfg",
      .s2 = "abcdefg",
      .want_result = 1});
  test_cases.emplace_back(TestCaseForStrDif{
      .name = "case 3",
      .s1 = "abcdfg",
      .s2 = "abcdef",
      .want_result = 2});
  test_cases.emplace_back(TestCaseForStrDif{
      .name = "bad case 1",
      .s1 = "",
      .s2 = "",
      .want_result = 0});
  test_cases.emplace_back(TestCaseForStrDif{
      .name = "bad case 3",
      .s1 = "",
      .s2 = "a",
      .want_result = 1});
  test_cases.emplace_back(TestCaseForStrDif{
      .name = "bad case 4",
      .s1 = "b",
      .s2 = "a",
      .want_result = 1});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = StrDif(
        test_cases[ii].s1,
        test_cases[ii].s2);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_ALGS_TEST, LongestSubStrWithoutDup) {
  struct TestCaseForLongestSubStrWithoutDup {
    std::string name;

    std::string s;

    std::pair<std::size_t, std::size_t> want_result;
  };
  std::vector<TestCaseForLongestSubStrWithoutDup> test_cases;

  test_cases.emplace_back(TestCaseForLongestSubStrWithoutDup{
      .name = "case 1",
      .s = "arabcacfr",
      .want_result = {1, 4}});
  test_cases.emplace_back(TestCaseForLongestSubStrWithoutDup{
      .name = "case 2",
      .s = "",
      .want_result = {0, 0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = LongestSubStrWithoutDup(
        test_cases[ii].s);
    EXPECT_EQ(ret.first, test_cases[ii].want_result.first)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(ret.second, test_cases[ii].want_result.second)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, Trim) {
  struct TestCaseForTrim {
    std::string name;

    std::string s;

    std::string want_result;
  };
  std::vector<TestCaseForTrim> test_cases;

  test_cases.emplace_back(TestCaseForTrim{
      .name = "case 1",
      .s = " testval  ",
      .want_result = "testval"});
  test_cases.emplace_back(TestCaseForTrim{
      .name = "case 2",
      .s = " ",
      .want_result = ""});
  test_cases.emplace_back(TestCaseForTrim{
      .name = "case 3",
      .s = "",
      .want_result = ""});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Trim(
        test_cases[ii].s);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(ret, test_cases[ii].s)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, ReplaceString) {
  struct TestCaseForReplaceString {
    std::string name;

    std::string str;
    std::string ov;
    std::string nv;

    std::string want_result;
  };
  std::vector<TestCaseForReplaceString> test_cases;

  test_cases.emplace_back(TestCaseForReplaceString{
      .name = "case 1",
      .str = "val1+val2+val3",
      .ov = "val1",
      .nv = "val10",
      .want_result = "val10+val2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      .name = "case 2",
      .str = "val1+val2+val3",
      .ov = "val2",
      .nv = "v2",
      .want_result = "val1+v2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      .name = "case 3",
      .str = "val1+val2+val3",
      .ov = "val3",
      .nv = "val4",
      .want_result = "val1+val2+val4"});
  test_cases.emplace_back(TestCaseForReplaceString{
      .name = "bad case 1",
      .str = "val1+val2+val3",
      .ov = "kkk",
      .nv = "ddd",
      .want_result = "val1+val2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      .name = "bad case 2",
      .str = "val1+val2+val3",
      .ov = "",
      .nv = "ddd",
      .want_result = "val1+val2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      .name = "bad case 3",
      .str = "val1+val2+val3",
      .ov = "+",
      .nv = "",
      .want_result = "val1val2val3"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = ReplaceString(
        test_cases[ii].str,
        test_cases[ii].ov,
        test_cases[ii].nv);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_STREQ(ret.c_str(), test_cases[ii].str.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, IsAlnumStr) {
  struct TestCaseForIsAlnumStr {
    std::string name;

    std::string str;

    bool want_result;
  };
  std::vector<TestCaseForIsAlnumStr> test_cases;

  test_cases.emplace_back(TestCaseForIsAlnumStr{
      .name = "case 1",
      .str = "123456789",
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsAlnumStr{
      .name = "case 2",
      .str = "123456789abcd",
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsAlnumStr{
      .name = "case 3",
      .str = "123456789..",
      .want_result = false});
  test_cases.emplace_back(TestCaseForIsAlnumStr{
      .name = "case 4",
      .str = "",
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = IsAlnumStr(
        test_cases[ii].str);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, SplitToMap) {
  struct TestCaseForSplitToMap {
    std::string name;

    std::string source;
    std::string vsep;
    std::string msep;
    bool trimempty;

    std::map<std::string, std::string> want_result;
  };
  std::vector<TestCaseForSplitToMap> test_cases;

  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "case 1",
      .source = "k1=v1&k2=v2&k3=v3&k4=v4",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "case 2",
      .source = "k1=v1& k2 =v2&k3= v3 &k4=v4",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "case 3",
      .source = "k1=v1&k2=v2&k3=v3&&k4=v4",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "case 4",
      .source = "k1=v1&k2=v2&k3=v3&k4=v4&k4=v4x",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4x"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "case 5",
      .source = "k1=v1&k2=v2&k3=v3&=& =v5&k6= &",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"", "v5"}, {"k6", ""}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "bad case 1",
      .source = "k1== v1&k2=v2&k3=v3",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "= v1"}, {"k2", "v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "bad case 2",
      .source = "k1=v1 = v11&k2=v2&k3=v3",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1 = v11"}, {"k2", "v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "bad case 3",
      .source = "&k1=v1&k2=v2&k3=v3",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "bad case 4",
      .source = "k0&k1=v1&k2=v2&k3=v3",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "bad case 5",
      .source = "k1=v1&xx &k2=v2&k3=v3",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "bad case 6",
      .source = "k1=v1&k2&==v2&k3=v3",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {{"k1", "v1"}, {"", "=v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      .name = "bad case 7",
      .source = "",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = {}});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = SplitToMap(
        test_cases[ii].source,
        test_cases[ii].vsep,
        test_cases[ii].msep,
        test_cases[ii].trimempty);
    EXPECT_TRUE(CheckMapEqual(ret, test_cases[ii].want_result))
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, JoinMap) {
  struct TestCaseForJoinMap {
    std::string name;

    std::map<std::string, std::string> m;
    std::string vsep;
    std::string msep;

    std::string want_result;
  };
  std::vector<TestCaseForJoinMap> test_cases;

  test_cases.emplace_back(TestCaseForJoinMap{
      .name = "case 1",
      .m = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}},
      .vsep = "&",
      .msep = "=",
      .want_result = "k1=v1&k2=v2&k3=v3&k4=v4"});
  test_cases.emplace_back(TestCaseForJoinMap{
      .name = "case 2",
      .m = {{"k1", "v1"}, {"k2", "v2"}, {"k3", ""}, {"", "v4"}},
      .vsep = "&",
      .msep = "=",
      .want_result = "=v4&k1=v1&k2=v2&k3="});
  test_cases.emplace_back(TestCaseForJoinMap{
      .name = "case 3",
      .m = {{"k1", "v1"}},
      .vsep = "&",
      .msep = "=",
      .want_result = "k1=v1"});
  test_cases.emplace_back(TestCaseForJoinMap{
      .name = "case 4",
      .m = {{"", ""}},
      .vsep = "&",
      .msep = "=",
      .want_result = "="});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = JoinMap(
        test_cases[ii].m,
        test_cases[ii].vsep,
        test_cases[ii].msep);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, GetValueFromStrKV) {
  struct TestCaseForGetValueFromStrKV {
    std::string name;

    std::string str;
    std::string key;
    std::string vsep;
    std::string msep;
    bool trimempty;

    std::string want_result;
  };
  std::vector<TestCaseForGetValueFromStrKV> test_cases;

  test_cases.emplace_back(TestCaseForGetValueFromStrKV{
      .name = "case 1",
      .str = "k1=v1&k2=v2&k3=v3&k4=v4",
      .key = "k1",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = "v1"});
  test_cases.emplace_back(TestCaseForGetValueFromStrKV{
      .name = "case 2",
      .str = "k1=v1&k2=v2&k3=v3&k4=v4",
      .key = "k3",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = "v3"});
  test_cases.emplace_back(TestCaseForGetValueFromStrKV{
      .name = "case 3",
      .str = "=v1&k2=v2&k3=v3&k4=v4",
      .key = "",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = "v1"});
  test_cases.emplace_back(TestCaseForGetValueFromStrKV{
      .name = "case 4",
      .str = "k1=v1&k2=v2&k3=v3&k4=v4",
      .key = "k5",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = ""});
  test_cases.emplace_back(TestCaseForGetValueFromStrKV{
      .name = "case 5",
      .str = "k1=v1&k2=v2&k2=v3&k2=v4",
      .key = "k2",
      .vsep = "&",
      .msep = "=",
      .trimempty = true,
      .want_result = "v2"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = GetValueFromStrKV(
        test_cases[ii].str,
        test_cases[ii].key,
        test_cases[ii].vsep,
        test_cases[ii].msep,
        test_cases[ii].trimempty);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, GetMapKeys) {
  struct TestCaseForGetMapKeys {
    std::string name;

    std::map<std::string, std::string> m;

    std::set<std::string> want_result;
  };
  std::vector<TestCaseForGetMapKeys> test_cases;

  test_cases.emplace_back(TestCaseForGetMapKeys{
      .name = "case 1",
      .m = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}},
      .want_result = {"k1", "k2", "k3", "k4"}});
  test_cases.emplace_back(TestCaseForGetMapKeys{
      .name = "case 2",
      .m = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"", "v4"}},
      .want_result = {"k1", "k2", "k3", ""}});
  test_cases.emplace_back(TestCaseForGetMapKeys{
      .name = "case 3",
      .m = {},
      .want_result = {}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = GetMapKeys(
        test_cases[ii].m);
    EXPECT_TRUE(CheckSetEqual(ret, test_cases[ii].want_result))
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, SplitToVec) {
  struct TestCaseForSplitToVec {
    std::string name;

    std::string source;
    std::string sep;
    bool trimempty;

    std::vector<std::string> want_result;
  };
  std::vector<TestCaseForSplitToVec> test_cases;

  test_cases.emplace_back(TestCaseForSplitToVec{
      .name = "case 1",
      .source = "v1,v2, v3,v4",
      .sep = ",",
      .trimempty = true,
      .want_result = {"v1", "v2", "v3", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      .name = "case 2",
      .source = "v1,v2, v3,v4",
      .sep = ",",
      .trimempty = false,
      .want_result = {"v1", "v2", " v3", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      .name = "case 3",
      .source = "v1,v2,, v3 , ,v4,",
      .sep = ",",
      .trimempty = true,
      .want_result = {"v1", "v2", "v3", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      .name = "case 4",
      .source = "v1,v2,, v3 , ,v4,",
      .sep = ",",
      .trimempty = false,
      .want_result = {"v1", "v2", " v3 ", " ", "v4"}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      .name = "bad case 1",
      .source = " ",
      .sep = ",",
      .trimempty = true,
      .want_result = {}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      .name = "bad case 2",
      .source = " ",
      .sep = ",",
      .trimempty = false,
      .want_result = {" "}});

  test_cases.emplace_back(TestCaseForSplitToVec{
      .name = "bad case 3",
      .source = " , ",
      .sep = ",",
      .trimempty = false,
      .want_result = {" ", " "}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = SplitToVec(
        test_cases[ii].source,
        test_cases[ii].sep,
        test_cases[ii].trimempty);
    ASSERT_TRUE(CheckVectorEqual(ret, test_cases[ii].want_result))
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, JoinVec) {
  struct TestCaseForJoinVec {
    std::string name;

    std::vector<std::string> vec;
    std::string sep;

    std::string want_result;
  };
  std::vector<TestCaseForJoinVec> test_cases;

  test_cases.emplace_back(TestCaseForJoinVec{
      .name = "case 1",
      .vec = {"v1", "v2", "v3", "v4"},
      .sep = "|",
      .want_result = "v1|v2|v3|v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      .name = "case 2",
      .vec = {"v1", "v2", "v3", "v4"},
      .sep = "",
      .want_result = "v1v2v3v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      .name = "case 3",
      .vec = {"", "", "", "v4"},
      .sep = "",
      .want_result = "v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      .name = "case 4",
      .vec = {"", "", "", ""},
      .sep = "",
      .want_result = ""});
  test_cases.emplace_back(TestCaseForJoinVec{
      .name = "case 5",
      .vec = {"", "", "", "v4"},
      .sep = ",",
      .want_result = ",,,v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      .name = "case 6",
      .vec = {"", "v2", "", "v4"},
      .sep = ",",
      .want_result = ",v2,,v4"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = JoinVec(
        test_cases[ii].vec,
        test_cases[ii].sep);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, CheckIfInList) {
  struct TestCaseForCheckIfInList {
    std::string name;

    std::string strlist;
    std::string key;
    std::string sep;
    bool trimempty;

    bool want_result;
  };
  std::vector<TestCaseForCheckIfInList> test_cases;

  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 1",
      .strlist = "123456",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 2",
      .strlist = "0123456",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 3",
      .strlist = "1234567",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 4",
      .strlist = "123456,",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 5",
      .strlist = ",123456",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 6",
      .strlist = "aaa,123456,bbb",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 7",
      .strlist = "aaa,0123456,bbb",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 8",
      .strlist = "aaa,1234567,bbb",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "case 9",
      .strlist = "aaa,1234567,123456,bbb",
      .key = "123456",
      .sep = ",",
      .trimempty = true,
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "bad case 1",
      .strlist = "123,456",
      .key = "123,456",
      .sep = ",",
      .trimempty = true,
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "bad case 2",
      .strlist = "aaa,,bbb",
      .key = "",
      .sep = ",",
      .trimempty = true,
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      .name = "bad case 3",
      .strlist = "aaa,bbb",
      .key = "aaa",
      .sep = "",
      .trimempty = false,
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CheckIfInList(
        test_cases[ii].strlist,
        test_cases[ii].key,
        test_cases[ii].sep,
        test_cases[ii].trimempty);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    ;
  }
}

TEST(STRING_UTIL_TEST, JoinSet) {
  struct TestCaseForJoinSet {
    std::string name;

    std::set<std::string> st;
    std::string sep;

    std::string want_result;
  };
  std::vector<TestCaseForJoinSet> test_cases;

  test_cases.emplace_back(TestCaseForJoinSet{
      .name = "case 1",
      .st = {"v1", "v2", "v3", "v4"},
      .sep = "|",
      .want_result = "v1|v2|v3|v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      .name = "case 2",
      .st = {"v1", "v2", "v3", "v4"},
      .sep = "",
      .want_result = "v1v2v3v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      .name = "case 3",
      .st = {"", "v4"},
      .sep = "",
      .want_result = "v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      .name = "case 4",
      .st = {""},
      .sep = "",
      .want_result = ""});
  test_cases.emplace_back(TestCaseForJoinSet{
      .name = "case 5",
      .st = {"", "v4"},
      .sep = ",",
      .want_result = ",v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      .name = "case 6",
      .st = {"", "v2", "v4"},
      .sep = ",",
      .want_result = ",v2,v4"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = JoinSet(
        test_cases[ii].st,
        test_cases[ii].sep);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, CmpVersion) {
  struct TestCaseForCmpVersion {
    std::string name;

    std::string ver1;
    std::string ver2;

    int want_result;
  };
  std::vector<TestCaseForCmpVersion> test_cases;

  test_cases.emplace_back(TestCaseForCmpVersion{
      .name = "case 1",
      .ver1 = "7.6.8.11020",
      .ver2 = "7.6.8",
      .want_result = 1});
  test_cases.emplace_back(TestCaseForCmpVersion{
      .name = "case 2",
      .ver1 = "7.6.8",
      .ver2 = "7.6.8",
      .want_result = 0});
  test_cases.emplace_back(TestCaseForCmpVersion{
      .name = "case 3",
      .ver1 = "7.6.8",
      .ver2 = "7.6.8.11020",
      .want_result = -1});
  test_cases.emplace_back(TestCaseForCmpVersion{
      .name = "bad case 1",
      .ver1 = "9.8.0",
      .ver2 = "9..8.0",
      .want_result = 0});
  test_cases.emplace_back(TestCaseForCmpVersion{
      .name = "bad case 2",
      .ver1 = "",
      .ver2 = "0.0.0",
      .want_result = -1});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CmpVersion(
        test_cases[ii].ver1,
        test_cases[ii].ver2);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }

  EXPECT_EQ(CheckVersionInside("7.6.8", "7.6.7", "7.6.9"), true);
  EXPECT_EQ(CheckVersionInside("7.6.8", "7.6.8", "7.6.8"), true);
  EXPECT_EQ(CheckVersionInside("7.6.6", "7.6.7", "7.6.9"), false);
}

TEST(STRING_UTIL_TEST, CheckVersionInside) {
  struct TestCaseForCheckVersionInside {
    std::string name;

    std::string ver;
    std::string start_ver;
    std::string end_ver;

    bool want_result;
  };
  std::vector<TestCaseForCheckVersionInside> test_cases;

  test_cases.emplace_back(TestCaseForCheckVersionInside{
      .name = "case 1",
      .ver = "7.6.8",
      .start_ver = "7.6.7",
      .end_ver = "7.6.9",
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      .name = "case 2",
      .ver = "7.6.8",
      .start_ver = "7.6.8",
      .end_ver = "7.6.8",
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      .name = "case 3",
      .ver = "7.6.5",
      .start_ver = "7.6.7",
      .end_ver = "7.6.9",
      .want_result = false});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      .name = "bad case 1",
      .ver = "7.6.8",
      .start_ver = "",
      .end_ver = "",
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      .name = "bad case 2",
      .ver = "0.0.0.0",
      .start_ver = "",
      .end_ver = "",
      .want_result = true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      .name = "bad case 3",
      .ver = "999.9.9.9",
      .start_ver = "",
      .end_ver = "",
      .want_result = true});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CheckVersionInside(
        test_cases[ii].ver,
        test_cases[ii].start_ver,
        test_cases[ii].end_ver);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(STRING_UTIL_TEST, Map_BASE) {
  std::map<std::string, std::string> test_map{{"k1", "v1"}, {"k2", ""}, {"", "v3"}};

  EXPECT_STREQ(GetMapItemWithDef(test_map, "k1").c_str(), "v1");
  EXPECT_STREQ(GetMapItemWithDef(test_map, "k2").c_str(), "");
  EXPECT_STREQ(GetMapItemWithDef(test_map, "").c_str(), "v3");
  EXPECT_STREQ(GetMapItemWithDef(test_map, "k4").c_str(), "");
  EXPECT_STREQ(GetMapItemWithDef(test_map, "k5", "default").c_str(), "default");

  string str1 = "=v5&k1=v1&k2=v2&k3=v3&k4=v4x&k6=";
  AddKV(str1, "k8", "v8");
  EXPECT_STREQ(str1.c_str(), "=v5&k1=v1&k2=v2&k3=v3&k4=v4x&k6=&k8=v8");
  string str2 = "";
  AddKV(str2, "k8", "v8");
  EXPECT_STREQ(str2.c_str(), "k8=v8");
}

TEST(URL_ENCODE_TEST, BASE) {
  string s = "http://abc123.com/aaa/bbbb?qa=1&qb=adf";

  string se = UrlEncode(s, false);
  EXPECT_STREQ(se.c_str(), "http%3a%2f%2fabc123.com%2faaa%2fbbbb%3fqa%3d1%26qb%3dadf");

  string sd = UrlDecode(se);
  EXPECT_STREQ(sd.c_str(), s.c_str());
}

TEST(TSTRING_TEST, BASE) {
  std::string s = "test123";

  std::wstring ws = ytlib::ToWString(s);
  EXPECT_STREQ(ws.c_str(), L"test123");

  std::string s1 = ytlib::ToString(ws);
  EXPECT_STREQ(s1.c_str(), "test123");
}

}  // namespace ytlib
