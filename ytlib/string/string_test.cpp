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
      "case 1",
      "abcdef abcdefg abcdefgh",
      "abcdef",
      0});
  test_cases.emplace_back(TestCaseForKMP{
      "case 2",
      "abcdef abcdefg abcdefgh",
      "abcdefg",
      7});
  test_cases.emplace_back(TestCaseForKMP{
      "case 3",
      "abcdef abcdefg abcdefgh",
      "abcdefgh",
      15});
  test_cases.emplace_back(TestCaseForKMP{
      "case 4",
      "abcdef abcdefg abcdefgh",
      "aaaa",
      23});
  test_cases.emplace_back(TestCaseForKMP{
      "bad case 1",
      "123",
      "",
      3});
  test_cases.emplace_back(TestCaseForKMP{
      "bad case 2",
      "123",
      "4",
      3});
  test_cases.emplace_back(TestCaseForKMP{
      "bad case 3",
      "123",
      "1234",
      3});
  test_cases.emplace_back(TestCaseForKMP{
      "bad case 4",
      "",
      "",
      0});

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
      "case 1",
      "abcdxfg",
      "abcdefg",
      1});
  test_cases.emplace_back(TestCaseForStrDif{
      "case 2",
      "abcdfg",
      "abcdefg",
      1});
  test_cases.emplace_back(TestCaseForStrDif{
      "case 3",
      "abcdfg",
      "abcdef",
      2});
  test_cases.emplace_back(TestCaseForStrDif{
      "bad case 1",
      "",
      "",
      0});
  test_cases.emplace_back(TestCaseForStrDif{
      "bad case 3",
      "",
      "a",
      1});
  test_cases.emplace_back(TestCaseForStrDif{
      "bad case 4",
      "b",
      "a",
      1});
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
      "case 1",
      "arabcacfr",
      {1, 4}});
  test_cases.emplace_back(TestCaseForLongestSubStrWithoutDup{
      "case 2",
      "",
      {0, 0}});

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
      "case 1",
      " testval  ",
      "testval"});
  test_cases.emplace_back(TestCaseForTrim{
      "case 2",
      " ",
      ""});
  test_cases.emplace_back(TestCaseForTrim{
      "case 3",
      "",
      ""});

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
      "case 1",
      "val1+val2+val3",
      "val1",
      "val10",
      "val10+val2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      "case 2",
      "val1+val2+val3",
      "val2",
      "v2",
      "val1+v2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      "case 3",
      "val1+val2+val3",
      "val3",
      "val4",
      "val1+val2+val4"});
  test_cases.emplace_back(TestCaseForReplaceString{
      "bad case 1",
      "val1+val2+val3",
      "kkk",
      "ddd",
      "val1+val2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      "bad case 2",
      "val1+val2+val3",
      "",
      "ddd",
      "val1+val2+val3"});
  test_cases.emplace_back(TestCaseForReplaceString{
      "bad case 3",
      "val1+val2+val3",
      "+",
      "",
      "val1val2val3"});

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
      "case 1",
      "123456789",
      true});
  test_cases.emplace_back(TestCaseForIsAlnumStr{
      "case 2",
      "123456789abcd",
      true});
  test_cases.emplace_back(TestCaseForIsAlnumStr{
      "case 3",
      "123456789..",
      false});
  test_cases.emplace_back(TestCaseForIsAlnumStr{
      "case 4",
      "",
      false});

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
      "case 1",
      "k1=v1&k2=v2&k3=v3&k4=v4",
      "&",
      "=",
      true,
      {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      "case 2",
      "k1=v1& k2 =v2&k3= v3 &k4=v4",
      "&",
      "=",
      true,
      {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      "case 3",
      "k1=v1&k2=v2&k3=v3&&k4=v4",
      "&",
      "=",
      true,
      {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      "case 4",
      "k1=v1&k2=v2&k3=v3&k4=v4&k4=v4x",
      "&",
      "=",
      true,
      {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4x"}}});
  test_cases.emplace_back(TestCaseForSplitToMap{
      "case 5",
      "k1=v1&k2=v2&k3=v3&k4=v4& =v5&k6= &",
      "&",
      "=",
      true,
      {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k4", "v4"}, {"", "v5"}, {"k6", ""}}});

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

TEST(STRING_UTIL_TEST, Map_BASE) {
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
      "case 1",
      {"v1", "v2", "v3", "v4"},
      "|",
      "v1|v2|v3|v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      "case 2",
      {"v1", "v2", "v3", "v4"},
      "",
      "v1v2v3v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      "case 3",
      {"", "", "", "v4"},
      "",
      "v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      "case 4",
      {"", "", "", ""},
      "",
      ""});
  test_cases.emplace_back(TestCaseForJoinVec{
      "case 5",
      {"", "", "", "v4"},
      ",",
      "v4"});
  test_cases.emplace_back(TestCaseForJoinVec{
      "case 6",
      {"", "v2", "", "v4"},
      ",",
      "v2,v4"});

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
      "case 1",
      "123456",
      "123456",
      ",",
      true,
      true});

  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 2",
      "0123456",
      "123456",
      ",",
      true,
      false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 3",
      "1234567",
      "123456",
      ",",
      true,
      false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 4",
      "123456,",
      "123456",
      ",",
      true,
      true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 5",
      ",123456",
      "123456",
      ",",
      true,
      true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 6",
      "aaa,123456,bbb",
      "123456",
      ",",
      true,
      true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 7",
      "aaa,0123456,bbb",
      "123456",
      ",",
      true,
      false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 8",
      "aaa,1234567,bbb",
      "123456",
      ",",
      true,
      false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "case 9",
      "aaa,1234567,123456,bbb",
      "123456",
      ",",
      true,
      true});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "bad case 1",
      "123,456",
      "123,456",
      ",",
      true,
      false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "bad case 2",
      "aaa,,bbb",
      "",
      ",",
      true,
      false});
  test_cases.emplace_back(TestCaseForCheckIfInList{
      "bad case 3",
      "aaa,bbb",
      "aaa",
      "",
      false,
      false});

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
      "case 1",
      {"v1", "v2", "v3", "v4"},
      "|",
      "v1|v2|v3|v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      "case 2",
      {"v1", "v2", "v3", "v4"},
      "",
      "v1v2v3v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      "case 3",
      {"", "", "", "v4"},
      "",
      "v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      "case 4",
      {"", "", "", ""},
      "",
      ""});
  test_cases.emplace_back(TestCaseForJoinSet{
      "case 5",
      {"", "", "", "v4"},
      ",",
      "v4"});
  test_cases.emplace_back(TestCaseForJoinSet{
      "case 6",
      {"", "v2", "", "v4"},
      ",",
      "v2,v4"});

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
      "case 1",
      "7.6.8.11020",
      "7.6.8",
      1});
  test_cases.emplace_back(TestCaseForCmpVersion{
      "case 2",
      "7.6.8",
      "7.6.8",
      0});
  test_cases.emplace_back(TestCaseForCmpVersion{
      "case 3",
      "7.6.8",
      "7.6.8.11020",
      -1});
  test_cases.emplace_back(TestCaseForCmpVersion{
      "bad case 1",
      "9.8.0",
      "9..8.0",
      0});
  test_cases.emplace_back(TestCaseForCmpVersion{
      "bad case 2",
      "",
      "0.0.0",
      -1});

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
      "case 1",
      "7.6.8",
      "7.6.7",
      "7.6.9",
      true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      "case 2",
      "7.6.8",
      "7.6.8",
      "7.6.8",
      true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      "case 3",
      "7.6.5",
      "7.6.7",
      "7.6.9",
      false});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      "bad case 1",
      "7.6.8",
      "",
      "",
      true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      "bad case 2",
      "0.0.0.0",
      "",
      "",
      true});
  test_cases.emplace_back(TestCaseForCheckVersionInside{
      "bad case 3",
      "999.9.9.9",
      "",
      "",
      true});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CheckVersionInside(
        test_cases[ii].ver,
        test_cases[ii].start_ver,
        test_cases[ii].end_ver);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
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
