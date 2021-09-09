#include <gtest/gtest.h>

#include "math_util.hpp"

#include "ytlib/misc/stl_util.hpp"

namespace ytlib {

TEST(MATH_UTIL_TEST, CountOne_test) {
  struct TestCaseForCountOne {
    std::string name;

    uint64_t n;

    uint8_t want_result;
  };
  std::vector<TestCaseForCountOne> test_cases;
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 1",
      .n = 0b0,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 2",
      .n = 0b1000,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 3",
      .n = 0b1001,
      .want_result = 2});
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 4",
      .n = 0b1010010001,
      .want_result = 4});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(CountOne(test_cases[ii].n), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(CountZero(test_cases[ii].n), 64 - test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, FindFirstOne_test) {
  struct TestCaseForFindFirstOne {
    std::string name;

    uint64_t val;
    uint8_t pos;

    uint8_t want_result;
  };
  std::vector<TestCaseForFindFirstOne> test_cases;
  test_cases.emplace_back(TestCaseForFindFirstOne{
      .name = "case 1",
      .val = 0b101000,
      .pos = 0,
      .want_result = 3});
  test_cases.emplace_back(TestCaseForFindFirstOne{
      .name = "case 2",
      .val = 0b101001,
      .pos = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForFindFirstOne{
      .name = "case 3",
      .val = 0b101001,
      .pos = 1,
      .want_result = 3});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = FindFirstOne(test_cases[ii].val, test_cases[ii].pos);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, IsPrime_test) {
  struct TestCaseForIsPrime {
    std::string name;

    uint64_t num;

    bool want_result;
  };
  std::vector<TestCaseForIsPrime> test_cases;

  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 1",
      .num = 0,
      .want_result = false});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 2",
      .num = 1,
      .want_result = false});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 3",
      .num = 2,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 4",
      .num = 3,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 5",
      .num = 4,
      .want_result = false});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 6",
      .num = 23,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 7",
      .num = 997,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 8",
      .num = 997 * 991,
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(IsPrime(test_cases[ii].num), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Gcd_test) {
  struct TestCaseForGcd {
    std::string name;

    uint64_t num1;
    uint64_t num2;

    uint64_t want_result;
  };
  std::vector<TestCaseForGcd> test_cases;

  test_cases.emplace_back(TestCaseForGcd{
      .name = "good case 1",
      .num1 = 42,
      .num2 = 30,
      .want_result = 6});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "good case 2",
      .num1 = 770,
      .num2 = 26,
      .want_result = 2});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "good case 3",
      .num1 = 121,
      .num2 = 132,
      .want_result = 11});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 1",
      .num1 = 1,
      .num2 = 1,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 2",
      .num1 = 1,
      .num2 = 2,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 3",
      .num1 = 1,
      .num2 = 0,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 4",
      .num1 = 0,
      .num2 = 0,
      .want_result = 1});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(Gcd(test_cases[ii].num1, test_cases[ii].num2), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Lcm_test) {
  struct TestCaseForLcm {
    std::string name;

    uint64_t num1;
    uint64_t num2;

    uint64_t want_result;
  };
  std::vector<TestCaseForLcm> test_cases;

  test_cases.emplace_back(TestCaseForLcm{
      .name = "good case 1",
      .num1 = 1,
      .num2 = 10,
      .want_result = 10});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "good case 2",
      .num1 = 2,
      .num2 = 5,
      .want_result = 10});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "good case 3",
      .num1 = 4,
      .num2 = 10,
      .want_result = 20});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "bad case 1",
      .num1 = 1,
      .num2 = 1,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "bad case 2",
      .num1 = 0,
      .num2 = 100,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "bad case 3",
      .num1 = 0,
      .num2 = 0,
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(Lcm(test_cases[ii].num1, test_cases[ii].num2), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Factoring2Vec_test) {
  struct TestCaseForFactoring2Vec {
    std::string name;

    uint64_t num;

    std::vector<uint64_t> want_result;
  };
  std::vector<TestCaseForFactoring2Vec> test_cases;
  test_cases.emplace_back(TestCaseForFactoring2Vec{
      .name = "case 1",
      .num = 60,
      .want_result = {2, 2, 3, 5}});
  test_cases.emplace_back(TestCaseForFactoring2Vec{
      .name = "case 2",
      .num = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCaseForFactoring2Vec{
      .name = "case 3",
      .num = 1,
      .want_result = {}});
  test_cases.emplace_back(TestCaseForFactoring2Vec{
      .name = "case 4",
      .num = 2,
      .want_result = {2}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Factoring2Vec(test_cases[ii].num);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Factoring2Map_test) {
  struct TestCaseForFactoring2Map {
    std::string name;

    uint64_t num;

    std::map<uint64_t, uint64_t> want_result;
  };
  std::vector<TestCaseForFactoring2Map> test_cases;
  test_cases.emplace_back(TestCaseForFactoring2Map{
      .name = "case 1",
      .num = 60,
      .want_result = {{2, 2}, {3, 1}, {5, 1}}});
  test_cases.emplace_back(TestCaseForFactoring2Map{
      .name = "case 2",
      .num = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCaseForFactoring2Map{
      .name = "case 3",
      .num = 1,
      .want_result = {}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Factoring2Map(test_cases[ii].num);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Mul_test) {
  struct TestCaseForMul {
    std::string name;

    uint64_t n;
    uint64_t m;

    uint64_t want_result;
  };
  std::vector<TestCaseForMul> test_cases;
  test_cases.emplace_back(TestCaseForMul{
      .name = "case 1",
      .n = 0,
      .m = 1,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForMul{
      .name = "case 2",
      .n = 5,
      .m = 1,
      .want_result = 120});
  test_cases.emplace_back(TestCaseForMul{
      .name = "case 3",
      .n = 9,
      .m = 5,
      .want_result = 15120});
  test_cases.emplace_back(TestCaseForMul{
      .name = "case 4",
      .n = 9,
      .m = 9,
      .want_result = 9});
  test_cases.emplace_back(TestCaseForMul{
      .name = "case 5",
      .n = 9,
      .m = 10,
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Mul(test_cases[ii].n, test_cases[ii].m);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Arn_test) {
  struct TestCaseForArn {
    std::string name;

    uint64_t n;
    uint64_t m;

    uint64_t want_result;
  };
  std::vector<TestCaseForArn> test_cases;
  test_cases.emplace_back(TestCaseForArn{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .want_result = 72});
  test_cases.emplace_back(TestCaseForArn{
      .name = "case 2",
      .n = 0,
      .m = 2,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForArn{
      .name = "case 3",
      .n = 2,
      .m = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForArn{
      .name = "case 4",
      .n = 9,
      .m = 1,
      .want_result = 9});
  test_cases.emplace_back(TestCaseForArn{
      .name = "case 5",
      .n = 4,
      .m = 4,
      .want_result = 24});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Arn(test_cases[ii].n, test_cases[ii].m);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, ArnRec_test) {
  struct TestCaseForArnRec {
    std::string name;

    uint64_t n;
    uint64_t m;
    uint64_t a;
    uint64_t mr;

    uint64_t want_result;
  };
  std::vector<TestCaseForArnRec> test_cases;
  test_cases.emplace_back(TestCaseForArnRec{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .a = Arn(9, 1),
      .mr = 1,
      .want_result = 72});
  test_cases.emplace_back(TestCaseForArnRec{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .a = Arn(9, 2),
      .mr = 2,
      .want_result = 72});
  test_cases.emplace_back(TestCaseForArnRec{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .a = Arn(9, 3),
      .mr = 3,
      .want_result = 72});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = ArnRec(test_cases[ii].n, test_cases[ii].m, test_cases[ii].a, test_cases[ii].mr);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Crn_test) {
  struct TestCaseForCrn {
    std::string name;

    uint64_t n;
    uint64_t m;

    uint64_t want_result;
  };
  std::vector<TestCaseForCrn> test_cases;
  test_cases.emplace_back(TestCaseForCrn{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .want_result = 36});
  test_cases.emplace_back(TestCaseForCrn{
      .name = "case 2",
      .n = 0,
      .m = 2,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForCrn{
      .name = "case 3",
      .n = 2,
      .m = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForCrn{
      .name = "case 4",
      .n = 9,
      .m = 1,
      .want_result = 9});
  test_cases.emplace_back(TestCaseForCrn{
      .name = "case 5",
      .n = 4,
      .m = 4,
      .want_result = 1});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Crn(test_cases[ii].n, test_cases[ii].m);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, CrnRec_test) {
  struct TestCaseForCrnRec {
    std::string name;

    uint64_t n;
    uint64_t m;
    uint64_t c;
    uint64_t mr;

    uint64_t want_result;
  };
  std::vector<TestCaseForCrnRec> test_cases;
  test_cases.emplace_back(TestCaseForCrnRec{
      .name = "case 1",
      .n = 9,
      .m = 2,
      .c = Crn(9, 1),
      .mr = 1,
      .want_result = 36});
  test_cases.emplace_back(TestCaseForCrnRec{
      .name = "case 2",
      .n = 9,
      .m = 2,
      .c = Crn(9, 2),
      .mr = 2,
      .want_result = 36});
  test_cases.emplace_back(TestCaseForCrnRec{
      .name = "case 3",
      .n = 9,
      .m = 2,
      .c = Crn(9, 3),
      .mr = 3,
      .want_result = 36});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = CrnRec(test_cases[ii].n, test_cases[ii].m, test_cases[ii].c, test_cases[ii].mr);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, SumAP_test) {
  struct TestCaseForSumAP {
    std::string name;

    double a1;
    double d;
    uint64_t n;

    double want_result;
  };
  std::vector<TestCaseForSumAP> test_cases;
  test_cases.emplace_back(TestCaseForSumAP{
      .name = "case 1",
      .a1 = 1.0,
      .d = 2.0,
      .n = 3,
      .want_result = 9.0});
  test_cases.emplace_back(TestCaseForSumAP{
      .name = "case 2",
      .a1 = 1.0,
      .d = 2.0,
      .n = 0,
      .want_result = 0.0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = SumAP(test_cases[ii].a1, test_cases[ii].d, test_cases[ii].n);
    EXPECT_DOUBLE_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, SumGP_test) {
  struct TestCaseForSumGP {
    std::string name;

    double a1;
    double q;
    uint64_t n;

    double want_result;
  };
  std::vector<TestCaseForSumGP> test_cases;
  test_cases.emplace_back(TestCaseForSumGP{
      .name = "case 1",
      .a1 = 2.0,
      .q = 0.0,
      .n = 10,
      .want_result = 0.0});
  test_cases.emplace_back(TestCaseForSumGP{
      .name = "case 2",
      .a1 = 0.0,
      .q = 2.0,
      .n = 10,
      .want_result = 0.0});
  test_cases.emplace_back(TestCaseForSumGP{
      .name = "case 3",
      .a1 = 0.0,
      .q = 0.0,
      .n = 10,
      .want_result = 0.0});
  test_cases.emplace_back(TestCaseForSumGP{
      .name = "case 4",
      .a1 = 2.0,
      .q = 1.0,
      .n = 10,
      .want_result = 20.0});
  test_cases.emplace_back(TestCaseForSumGP{
      .name = "case 5",
      .a1 = 2.0,
      .q = 3.0,
      .n = 4,
      .want_result = 80.0});
  test_cases.emplace_back(TestCaseForSumGP{
      .name = "case 6",
      .a1 = 2.0,
      .q = 3.0,
      .n = 0,
      .want_result = 0.0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = SumGP(test_cases[ii].a1, test_cases[ii].q, test_cases[ii].n);
    EXPECT_DOUBLE_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Pow_test) {
  struct TestCaseForPow {
    std::string name;

    double value;
    uint64_t n;

    double want_result;
  };
  std::vector<TestCaseForPow> test_cases;
  test_cases.emplace_back(TestCaseForPow{
      .name = "case 1",
      .value = 0.0,
      .n = 1,
      .want_result = 0.0});
  test_cases.emplace_back(TestCaseForPow{
      .name = "case 2",
      .value = 1.0,
      .n = 0,
      .want_result = 1.0});
  test_cases.emplace_back(TestCaseForPow{
      .name = "case 3",
      .value = 2.0,
      .n = 1,
      .want_result = 2.0});
  test_cases.emplace_back(TestCaseForPow{
      .name = "case 4",
      .value = 2.0,
      .n = 10,
      .want_result = 1024.0});
  test_cases.emplace_back(TestCaseForPow{
      .name = "case 5",
      .value = 0.0,
      .n = 0,
      .want_result = 0.0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Pow(test_cases[ii].value, test_cases[ii].n);
    EXPECT_DOUBLE_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
