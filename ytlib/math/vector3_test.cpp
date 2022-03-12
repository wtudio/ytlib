#include <gtest/gtest.h>

#include "vector3.hpp"

namespace ytlib {

TEST(VECTOR3_TEST, Len_test) {
  struct TestCase {
    std::string name;

    Vector3<float> vec;

    float want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .vec = {0, 0, 0},
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .vec = {1.26, 3.55, 9.08},
      .want_result = 9.8303852});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_FLOAT_EQ(test_cases[ii].vec.Len(), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(VECTOR3_TEST, Distance_test) {
  struct TestCase {
    std::string name;

    Vector3<float> vec1;
    Vector3<float> vec2;

    float want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .vec1 = {0, 0, 0},
      .vec2 = {0, 0, 0},
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .vec1 = {1.26, 3.55, 9.08},
      .vec2 = {0, 0, 0},
      .want_result = 9.8303852});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .vec1 = {1.26, 3.55, 9.08},
      .vec2 = {2.59, -4.8, 3.2},
      .want_result = 10.298825});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_FLOAT_EQ(test_cases[ii].vec1.Distance(test_cases[ii].vec2), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_FLOAT_EQ(Vector3<float>::Distance(test_cases[ii].vec1, test_cases[ii].vec2), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
