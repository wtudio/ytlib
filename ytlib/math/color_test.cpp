#include <gtest/gtest.h>

#include "color.hpp"

namespace ytlib {

TEST(COLOR_TEST, rgb2hsb_test) {
  struct TestCase {
    std::string name;

    std::vector<uint8_t> rgb;

    std::vector<float> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .rgb = {0, 0, 0},
      .want_result = {0.0, 0.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .rgb = {255, 255, 255},
      .want_result = {0.0, 0.0, 1.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .rgb = {204, 98, 106},
      .want_result = {355.47171, 0.519608, 0.8}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = rgb2hsb(cur_test_case.rgb);
    EXPECT_EQ(ret.size(), 3)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_FLOAT_EQ(ret[0], cur_test_case.want_result[0])
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_FLOAT_EQ(ret[1], cur_test_case.want_result[1])
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_FLOAT_EQ(ret[2], cur_test_case.want_result[2])
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(COLOR_TEST, hsb2rgb_test) {
  struct TestCase {
    std::string name;

    std::vector<float> hsb;

    std::vector<uint8_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .hsb = {0.0, 0.0, 0.0},
      .want_result = {0, 0, 0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .hsb = {0.0, 0.0, 1.0},
      .want_result = {255, 255, 255}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .hsb = {355.47171, 0.519608, 0.8},
      .want_result = {204, 98, 106}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = hsb2rgb(cur_test_case.hsb);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}
}  // namespace ytlib
