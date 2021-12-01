#include <gtest/gtest.h>

#include "string_algs.hpp"

namespace ytlib {

TEST(STRING_ALGS_TEST, KMP_test) {
  struct TestCaseForKMP {
    std::string name;

    std::string ss;
    std::string ps;

    size_t want_result;
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

TEST(STRING_ALGS_TEST, StrDif_test) {
  struct TestCaseForStrDif {
    std::string name;

    std::string s1;
    std::string s2;

    size_t want_result;
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

TEST(STRING_ALGS_TEST, LongestSubStrWithoutDup_test) {
  struct TestCaseForLongestSubStrWithoutDup {
    std::string name;

    std::string s;

    std::pair<size_t, size_t> want_result;
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

}  // namespace ytlib
