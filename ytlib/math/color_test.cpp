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
  test_cases.emplace_back(TestCase{
      .name = "case 4: pure red",
      .rgb = {255, 0, 0},
      .want_result = {0.0, 1.0, 1.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 5: pure green",
      .rgb = {0, 255, 0},
      .want_result = {120.0, 1.0, 1.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 6: pure blue",
      .rgb = {0, 0, 255},
      .want_result = {240.0, 1.0, 1.0}});

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
TEST(COLOR_TEST, rgb2hsb_exception_test) {
  EXPECT_THROW(rgb2hsb({}), std::logic_error);
  EXPECT_THROW(rgb2hsb({1, 2}), std::logic_error);
  EXPECT_THROW(rgb2hsb({1, 2, 3, 4}), std::logic_error);
}

TEST(COLOR_TEST, hsb2rgb_exception_test) {
  EXPECT_THROW(hsb2rgb({}), std::logic_error);
  EXPECT_THROW(hsb2rgb({-1.0, 0.5, 0.5}), std::logic_error);
  EXPECT_THROW(hsb2rgb({361.0, 0.5, 0.5}), std::logic_error);
  EXPECT_THROW(hsb2rgb({180.0, -0.1, 0.5}), std::logic_error);
  EXPECT_THROW(hsb2rgb({180.0, 0.5, 1.1}), std::logic_error);
}

}  // namespace ytlib
