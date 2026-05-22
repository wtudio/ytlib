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
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_FLOAT_EQ(cur_test_case.vec.Len(), cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_FLOAT_EQ(cur_test_case.vec1.Distance(cur_test_case.vec2), cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_FLOAT_EQ(Vector3<float>::Distance(cur_test_case.vec1, cur_test_case.vec2), cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(VECTOR3_TEST, Add_test) {
  Vector3<double> a(1.0, 2.0, 3.0);
  Vector3<double> b(4.0, 5.0, 6.0);

  auto c = a + b;
  EXPECT_EQ(c, Vector3<double>(5.0, 7.0, 9.0));

  a += b;
  EXPECT_EQ(a, Vector3<double>(5.0, 7.0, 9.0));
}

TEST(VECTOR3_TEST, Sub_test) {
  Vector3<double> a(4.0, 5.0, 6.0);
  Vector3<double> b(1.0, 2.0, 3.0);

  auto c = a - b;
  EXPECT_EQ(c, Vector3<double>(3.0, 3.0, 3.0));

  a -= b;
  EXPECT_EQ(a, Vector3<double>(3.0, 3.0, 3.0));
}

TEST(VECTOR3_TEST, MultiplyScalar_test) {
  Vector3<double> a(1.0, 2.0, 3.0);

  auto c = a * 2.0;
  EXPECT_EQ(c, Vector3<double>(2.0, 4.0, 6.0));

  a *= 2.0;
  EXPECT_EQ(a, Vector3<double>(2.0, 4.0, 6.0));
}

TEST(VECTOR3_TEST, DivideScalar_test) {
  Vector3<double> a(2.0, 4.0, 6.0);

  auto c = a / 2.0;
  EXPECT_EQ(c, Vector3<double>(1.0, 2.0, 3.0));

  a /= 2.0;
  EXPECT_EQ(a, Vector3<double>(1.0, 2.0, 3.0));
}

TEST(VECTOR3_TEST, Negate_test) {
  Vector3<double> a(1.0, -2.0, 3.0);
  auto b = -a;
  EXPECT_EQ(b, Vector3<double>(-1.0, 2.0, -3.0));
}

TEST(VECTOR3_TEST, Equality_test) {
  Vector3<double> a(1.0, 2.0, 3.0);
  Vector3<double> b(1.0, 2.0, 3.0);
  Vector3<double> c(1.0, 2.0, 4.0);

  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
  EXPECT_TRUE(a != c);
  EXPECT_FALSE(a == c);
}

TEST(VECTOR3_TEST, ostream_test) {
  Vector3<double> a(1.5, 2.5, 3.5);
  std::stringstream ss;
  ss << a;
  EXPECT_EQ(ss.str(), "(1.5, 2.5, 3.5)");
}

}  // namespace ytlib
