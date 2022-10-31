#include <gtest/gtest.h>

#include "math_util.hpp"

namespace ytlib {

TEST(MATH_UTIL_TEST, CountOne_test) {
  struct TestCase {
    std::string name;

    uint64_t n;

    uint8_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 0b0,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .n = 0b1000,
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .n = 0b1001,
      .want_result = 2});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .n = 0b1010010001,
      .want_result = 4});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_EQ(CountOne(cur_test_case.n), cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_EQ(CountZero(cur_test_case.n), 64 - cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, FindFirstOne_test) {
  struct TestCase {
    std::string name;

    uint64_t val;
    uint8_t pos;

    uint8_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .val = 0b101000,
      .pos = 0,
      .want_result = 3});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .val = 0b101001,
      .pos = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .val = 0b101001,
      .pos = 1,
      .want_result = 3});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = FindFirstOne(cur_test_case.val, cur_test_case.pos);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, IsPrime_test) {
  struct TestCase {
    std::string name;

    uint64_t num;

    bool want_result;
  };
  std::vector<TestCase> test_cases;

  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = 0,
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = 1,
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = 2,
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num = 3,
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .num = 4,
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .num = 23,
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .num = 997,
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 8",
      .num = 997 * 991,
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_EQ(IsPrime(cur_test_case.num), cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Gcd_test) {
  struct TestCase {
    std::string name;

    uint64_t num1;
    uint64_t num2;

    uint64_t want_result;
  };
  std::vector<TestCase> test_cases;

  test_cases.emplace_back(TestCase{
      .name = "good case 1",
      .num1 = 42,
      .num2 = 30,
      .want_result = 6});
  test_cases.emplace_back(TestCase{
      .name = "good case 2",
      .num1 = 770,
      .num2 = 26,
      .want_result = 2});
  test_cases.emplace_back(TestCase{
      .name = "good case 3",
      .num1 = 121,
      .num2 = 132,
      .want_result = 11});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .num1 = 1,
      .num2 = 1,
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "bad case 2",
      .num1 = 1,
      .num2 = 2,
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "bad case 3",
      .num1 = 1,
      .num2 = 0,
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "bad case 4",
      .num1 = 0,
      .num2 = 0,
      .want_result = 1});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_EQ(Gcd(cur_test_case.num1, cur_test_case.num2), cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Lcm_test) {
  struct TestCase {
    std::string name;

    uint64_t num1;
    uint64_t num2;

    uint64_t want_result;
  };
  std::vector<TestCase> test_cases;

  test_cases.emplace_back(TestCase{
      .name = "good case 1",
      .num1 = 1,
      .num2 = 10,
      .want_result = 10});
  test_cases.emplace_back(TestCase{
      .name = "good case 2",
      .num1 = 2,
      .num2 = 5,
      .want_result = 10});
  test_cases.emplace_back(TestCase{
      .name = "good case 3",
      .num1 = 4,
      .num2 = 10,
      .want_result = 20});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .num1 = 1,
      .num2 = 1,
      .want_result = 1});
  test_cases.emplace_back(TestCase{
      .name = "bad case 2",
      .num1 = 0,
      .num2 = 100,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "bad case 3",
      .num1 = 0,
      .num2 = 0,
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_EQ(Lcm(cur_test_case.num1, cur_test_case.num2), cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Factoring2Vec_test) {
  struct TestCase {
    std::string name;

    uint64_t num;

    std::vector<uint64_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = 60,
      .want_result = {2, 2, 3, 5}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = 1,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num = 2,
      .want_result = {2}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Factoring2Vec(cur_test_case.num);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Factoring2Map_test) {
  struct TestCase {
    std::string name;

    uint64_t num;

    std::map<uint64_t, uint64_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = 60,
      .want_result = {{2, 2}, {3, 1}, {5, 1}}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = 1,
      .want_result = {}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Factoring2Map(cur_test_case.num);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Mul_test) {
  struct TestCase {
    std::string name;

    uint64_t n;
    uint64_t m;

    uint64_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 0,
      .m = 1,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .n = 5,
      .m = 1,
      .want_result = 120});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .n = 9,
      .m = 5,
      .want_result = 15120});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .n = 9,
      .m = 9,
      .want_result = 9});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .n = 9,
      .m = 10,
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Mul(cur_test_case.n, cur_test_case.m);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Arn_test) {
  struct TestCase {
    std::string name;

    uint64_t n;
    uint64_t m;

    uint64_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .want_result = 72});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .n = 0,
      .m = 2,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .n = 2,
      .m = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .n = 9,
      .m = 1,
      .want_result = 9});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .n = 4,
      .m = 4,
      .want_result = 24});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Arn(cur_test_case.n, cur_test_case.m);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, ArnRec_test) {
  struct TestCase {
    std::string name;

    uint64_t n;
    uint64_t m;
    uint64_t a;
    uint64_t mr;

    uint64_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .a = Arn(9, 1),
      .mr = 1,
      .want_result = 72});
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .a = Arn(9, 2),
      .mr = 2,
      .want_result = 72});
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .a = Arn(9, 3),
      .mr = 3,
      .want_result = 72});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = ArnRec(cur_test_case.n, cur_test_case.m, cur_test_case.a, cur_test_case.mr);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Crn_test) {
  struct TestCase {
    std::string name;

    uint64_t n;
    uint64_t m;

    uint64_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .want_result = 36});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .n = 0,
      .m = 2,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .n = 2,
      .m = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .n = 9,
      .m = 1,
      .want_result = 9});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .n = 4,
      .m = 4,
      .want_result = 1});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Crn(cur_test_case.n, cur_test_case.m);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, CrnRec_test) {
  struct TestCase {
    std::string name;

    uint64_t n;
    uint64_t m;
    uint64_t c;
    uint64_t mr;

    uint64_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .c = Crn(9, 1),
      .mr = 1,
      .want_result = 36});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .n = 9,
      .m = 2,
      .c = Crn(9, 2),
      .mr = 2,
      .want_result = 36});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .n = 9,
      .m = 2,
      .c = Crn(9, 3),
      .mr = 3,
      .want_result = 36});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = CrnRec(cur_test_case.n, cur_test_case.m, cur_test_case.c, cur_test_case.mr);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, SumAP_test) {
  struct TestCase {
    std::string name;

    double a1;
    double d;
    uint64_t n;

    double want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a1 = 1.0,
      .d = 2.0,
      .n = 3,
      .want_result = 9.0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a1 = 1.0,
      .d = 2.0,
      .n = 0,
      .want_result = 0.0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = SumAP(cur_test_case.a1, cur_test_case.d, cur_test_case.n);
    EXPECT_DOUBLE_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, SumGP_test) {
  struct TestCase {
    std::string name;

    double a1;
    double q;
    uint64_t n;

    double want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a1 = 2.0,
      .q = 0.0,
      .n = 10,
      .want_result = 0.0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a1 = 0.0,
      .q = 2.0,
      .n = 10,
      .want_result = 0.0});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .a1 = 0.0,
      .q = 0.0,
      .n = 10,
      .want_result = 0.0});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .a1 = 2.0,
      .q = 1.0,
      .n = 10,
      .want_result = 20.0});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .a1 = 2.0,
      .q = 3.0,
      .n = 4,
      .want_result = 80.0});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .a1 = 2.0,
      .q = 3.0,
      .n = 0,
      .want_result = 0.0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = SumGP(cur_test_case.a1, cur_test_case.q, cur_test_case.n);
    EXPECT_DOUBLE_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Pow_test) {
  struct TestCase {
    std::string name;

    double value;
    uint64_t n;

    double want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .value = 0.0,
      .n = 1,
      .want_result = 0.0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .value = 1.0,
      .n = 0,
      .want_result = 1.0});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .value = 2.0,
      .n = 1,
      .want_result = 2.0});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .value = 2.0,
      .n = 10,
      .want_result = 1024.0});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .value = 0.0,
      .n = 0,
      .want_result = 0.0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Pow(cur_test_case.value, cur_test_case.n);
    EXPECT_DOUBLE_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

}  // namespace ytlib
