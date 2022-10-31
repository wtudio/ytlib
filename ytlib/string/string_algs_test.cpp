#include <gtest/gtest.h>

#include "string_algs.hpp"

namespace ytlib {

TEST(STRING_ALGS_TEST, KMP_test) {
  struct TestCase {
    std::string name;

    std::string ss;
    std::string ps;

    size_t want_result;
  };
  std::vector<TestCase> test_cases;

  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "abcdef",
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "abcdefg",
      .want_result = 7});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "abcdefgh",
      .want_result = 15});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .ss = "abcdef abcdefg abcdefgh",
      .ps = "aaaa",
      .want_result = 23});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .ss = "123",
      .ps = "",
      .want_result = 3});
  test_cases.emplace_back(TestCase{
      .name = "bad case 2",
      .ss = "123",
      .ps = "4",
      .want_result = 3});
  test_cases.emplace_back(TestCase{
      .name = "bad case 3",
      .ss = "123",
      .ps = "1234",
      .want_result = 3});
  test_cases.emplace_back(TestCase{
      .name = "bad case 4",
      .ss = "",
      .ps = "",
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = KMP(
        cur_test_case.ss,
        cur_test_case.ps);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(STRING_ALGS_TEST, StrDif_test) {
  struct TestCase {
    std::string name;

    std::string s1;
    std::string s2;

    size_t want_result;
  };
  std::vector<TestCase> test_cases;

  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .s1 = "abcdxfg",
      .s2 = "abcdefg",
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .s1 = "abcdfg",
      .s2 = "abcdefg",
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .s1 = "abcdfg",
      .s2 = "abcdef",
      .want_result = 2});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .s1 = "",
      .s2 = "",
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "bad case 3",
      .s1 = "",
      .s2 = "a",
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "bad case 4",
      .s1 = "b",
      .s2 = "a",
      .want_result = 1});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = StrDif(
        cur_test_case.s1,
        cur_test_case.s2);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(STRING_ALGS_TEST, LongestSubStrWithoutDup_test) {
  struct TestCase {
    std::string name;

    std::string s;

    std::pair<size_t, size_t> want_result;
  };
  std::vector<TestCase> test_cases;

  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .s = "arabcacfr",
      .want_result = {1, 4}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .s = "",
      .want_result = {0, 0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = LongestSubStrWithoutDup(
        cur_test_case.s);
    EXPECT_EQ(ret.first, cur_test_case.want_result.first)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_EQ(ret.second, cur_test_case.want_result.second)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

}  // namespace ytlib
