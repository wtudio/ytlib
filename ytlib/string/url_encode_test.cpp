#include <gtest/gtest.h>

#include "url_encode.hpp"

namespace ytlib {

TEST(URL_ENCODE_TEST, UrlEncode_test) {
  struct TestCaseForUrlEncode {
    std::string name;

    std::string str;
    bool up;

    std::string want_result;
  };
  std::vector<TestCaseForUrlEncode> test_cases;

  test_cases.emplace_back(TestCaseForUrlEncode{
      .name = "case 1",
      .str = "",
      .up = true,
      .want_result = ""});
  test_cases.emplace_back(TestCaseForUrlEncode{
      .name = "case 2",
      .str = "http://abc123.com/aaa/bbbb?qa=1&qb=adf",
      .up = true,
      .want_result = "http%3A%2F%2Fabc123.com%2Faaa%2Fbbbb%3Fqa%3D1%26qb%3Dadf"});
  test_cases.emplace_back(TestCaseForUrlEncode{
      .name = "case 3",
      .str = "http://abc123.com/aaa/bbbb?qa=1&qb=adf",
      .up = false,
      .want_result = "http%3a%2f%2fabc123.com%2faaa%2fbbbb%3fqa%3d1%26qb%3dadf"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = UrlEncode(
        test_cases[ii].str,
        test_cases[ii].up);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(URL_ENCODE_TEST, UrlDecode_test) {
  struct TestCaseForUrlDecode {
    std::string name;

    std::string str;

    std::string want_result;
  };
  std::vector<TestCaseForUrlDecode> test_cases;

  test_cases.emplace_back(TestCaseForUrlDecode{
      .name = "case 1",
      .str = "",
      .want_result = ""});
  test_cases.emplace_back(TestCaseForUrlDecode{
      .name = "case 2",
      .str = "http%3A%2F%2Fabc123.com%2Faaa%2Fbbbb%3Fqa%3D1%26qb%3Dadf",
      .want_result = "http://abc123.com/aaa/bbbb?qa=1&qb=adf"});
  test_cases.emplace_back(TestCaseForUrlDecode{
      .name = "case 3",
      .str = "http%3a%2f%2fabc123.com%2faaa%2fbbbb%3fqa%3d1%26qb%3dadf",
      .want_result = "http://abc123.com/aaa/bbbb?qa=1&qb=adf"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = UrlDecode(test_cases[ii].str);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
