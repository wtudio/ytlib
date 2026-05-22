#include <gtest/gtest.h>

#include "base64_encode.hpp"

namespace ytlib {

TEST(BASE64_ENCODE_TEST, Base64Encode) {
  struct TestCase {
    std::string name;

    std::string data;

    std::string want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .data = "",
      .want_result = ""});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .data = "a",
      .want_result = "YQ=="});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .data = "ab",
      .want_result = "YWI="});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .data = "abc",
      .want_result = "YWJj"});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .data = "abcd",
      .want_result = "YWJjZA=="});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .data = "hello world",
      .want_result = "aGVsbG8gd29ybGQ="});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Base64Encode(cur_test_case.data);
    EXPECT_STREQ(ret.c_str(), cur_test_case.want_result.c_str())
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BASE64_ENCODE_TEST, Base64Decode) {
  struct TestCase {
    std::string name;

    std::string data;

    std::string want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .data = "",
      .want_result = ""});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .data = "YQ==",
      .want_result = "a"});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .data = "YWI=",
      .want_result = "ab"});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .data = "YWJj",
      .want_result = "abc"});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .data = "YWJjZA==",
      .want_result = "abcd"});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .data = "aGVsbG8gd29ybGQ=",
      .want_result = "hello world"});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Base64Decode(cur_test_case.data);
    EXPECT_STREQ(ret.c_str(), cur_test_case.want_result.c_str())
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }

  EXPECT_THROW(Base64Decode("abc"), std::runtime_error);
  EXPECT_THROW(Base64Decode("!!!"), std::runtime_error);
}

}  // namespace ytlib
