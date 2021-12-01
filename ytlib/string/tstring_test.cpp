#include <gtest/gtest.h>

#include "tstring.hpp"

namespace ytlib {

TEST(TSTRING_TEST, BASE_test) {
  struct TestCaseForTSTRING {
    std::string name;

    std::string s;
    std::wstring ws;
  };
  std::vector<TestCaseForTSTRING> test_cases;

  test_cases.emplace_back(TestCaseForTSTRING{
      .name = "case 1",
      .s = "",
      .ws = L""});
  test_cases.emplace_back(TestCaseForTSTRING{
      .name = "case 2",
      .s = "test123",
      .ws = L"test123"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::wstring ret1 = ytlib::ToWString(test_cases[ii].s);
    EXPECT_STREQ(ret1.c_str(), test_cases[ii].ws.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    std::string ret2 = ytlib::ToString(test_cases[ii].ws);
    EXPECT_STREQ(ret2.c_str(), test_cases[ii].s.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
