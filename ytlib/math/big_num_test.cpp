#include <gtest/gtest.h>

#include "big_num.hpp"

namespace ytlib {

TEST(BIG_NUM_TEST, Construct_test) {
  struct TestCase {
    std::string name;

    BigNum num;

    bool want_symbol;
    std::vector<uint32_t> want_content;
    uint32_t want_base;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = BigNum(),
      .want_symbol = true,
      .want_content = {0},
      .want_base = UINT32_MAX});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = BigNum(0, 0, 0),
      .want_symbol = true,
      .want_content = {0},
      .want_base = UINT32_MAX});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = BigNum(123, 10, 0),
      .want_symbol = true,
      .want_content = {3, 2, 1},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num = BigNum(-123, 10, 0),
      .want_symbol = false,
      .want_content = {3, 2, 1},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .num = BigNum(0, 10, 2),
      .want_symbol = true,
      .want_content = {0},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .num = BigNum(123, 10, 2),
      .want_symbol = true,
      .want_content = {0, 0, 3, 2, 1},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .num = BigNum("10110", BigNum::BaseType::BIN, 2),
      .want_symbol = true,
      .want_content = {0, 1, 1, 0, 1},
      .want_base = 2});
  test_cases.emplace_back(TestCase{
      .name = "case 8",
      .num = BigNum("-12345", BigNum::BaseType::OCT, 0),
      .want_symbol = false,
      .want_content = {5, 4, 3, 2, 1},
      .want_base = 8});
  test_cases.emplace_back(TestCase{
      .name = "case 9",
      .num = BigNum("987", BigNum::BaseType::DEC, 0),
      .want_symbol = true,
      .want_content = {7, 8, 9},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 10",
      .num = BigNum("-0ABc9", BigNum::BaseType::HEX, 0),
      .want_symbol = false,
      .want_content = {9, 12, 11, 10},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 11",
      .num = BigNum("", BigNum::BaseType::HEX, 0),
      .want_symbol = true,
      .want_content = {0},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 12",
      .num = BigNum("-", BigNum::BaseType::HEX, 0),
      .want_symbol = false,
      .want_content = {0},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 13",
      .num = BigNum("-000kkk", BigNum::BaseType::HEX, 0),
      .want_symbol = false,
      .want_content = {0},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 14",
      .num = BigNum("-9abc@123", BigNum::BaseType::HEX, 0),
      .want_symbol = false,
      .want_content = {12, 11, 10, 9},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 15",
      .num = BigNum("+009abc@123", BigNum::BaseType::DEC, 0),
      .want_symbol = true,
      .want_content = {9},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 16",
      .num = BigNum::AssignU64(static_cast<uint64_t>(UINT32_MAX - 1) * (UINT32_MAX - 1), false),
      .want_symbol = false,
      .want_content = {1, UINT32_MAX - 2},
      .want_base = UINT32_MAX});
  test_cases.emplace_back(TestCase{
      .name = "case 17",
      .num = BigNum("987"),
      .want_symbol = true,
      .want_content = {987},
      .want_base = UINT32_MAX});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_EQ(cur_test_case.num.Symbol(), cur_test_case.want_symbol)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_EQ(cur_test_case.num.Content(), cur_test_case.want_content)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_EQ(cur_test_case.num.Base(), cur_test_case.want_base)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, ReBase_test) {
  struct TestCase {
    std::string name;

    BigNum num;
    uint32_t base;

    bool want_symbol;
    std::vector<uint32_t> want_content;
    uint32_t want_base;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = BigNum(123, 10),
      .base = 1,
      .want_symbol = true,
      .want_content = {3, 2, 1},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = BigNum(123, 10),
      .base = 10,
      .want_symbol = true,
      .want_content = {3, 2, 1},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = BigNum(110, 10),
      .base = 8,
      .want_symbol = true,
      .want_content = {6, 5, 1},
      .want_base = 8});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num = BigNum(-123, 10),
      .base = 100,
      .want_symbol = false,
      .want_content = {23, 1},
      .want_base = 100});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .num = BigNum(-123, 10),
      .base = 1000,
      .want_symbol = false,
      .want_content = {123},
      .want_base = 1000});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .num = BigNum(0, 10),
      .base = 8,
      .want_symbol = true,
      .want_content = {0},
      .want_base = 8});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .num = BigNum(UINT32_MAX, UINT32_MAX),
      .base = 2,
      .want_symbol = true,
      .want_content = std::vector<uint32_t>(32, 1),
      .want_base = 2});
  test_cases.emplace_back(TestCase{
      .name = "case 8",
      .num = BigNum(UINT32_MAX, 2),
      .base = UINT32_MAX,
      .want_symbol = true,
      .want_content = {0, 1},
      .want_base = UINT32_MAX});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    cur_test_case.num.ReBase(cur_test_case.base);
    EXPECT_EQ(cur_test_case.num.Symbol(), cur_test_case.want_symbol)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_EQ(cur_test_case.num.Content(), cur_test_case.want_content)
        << "Test " << cur_test_case.name << " failed, index " << ii;
    EXPECT_EQ(cur_test_case.num.Base(), cur_test_case.want_base)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, MISC_test) {
  BigNum n1;
  EXPECT_TRUE(n1.Empty());
  EXPECT_FALSE(n1);

  BigNum n2(1);
  EXPECT_FALSE(n2.Empty());
  EXPECT_TRUE(n2);

  n2.Clear();
  EXPECT_TRUE(n2.Empty());

  BigNum n3(-123);
  EXPECT_EQ(BigNum::Abs(n3), BigNum(123));
  BigNum n4(123);
  EXPECT_EQ(abs(n4), BigNum(123));

  std::swap(n3, n4);
  EXPECT_EQ(n3, BigNum(123));
  EXPECT_EQ(n4, BigNum(-123));
}

TEST(BIG_NUM_TEST, EQUAL_test) {
  struct TestCase {
    std::string name;

    BigNum num1;
    BigNum num2;

    bool want_equal;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num1 = BigNum("", BigNum::BaseType::HEX),
      .num2 = BigNum("-", BigNum::BaseType::HEX),
      .want_equal = true});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num1 = BigNum("123", BigNum::BaseType::HEX),
      .num2 = BigNum("", BigNum::BaseType::HEX),
      .want_equal = false});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num1 = BigNum("123", BigNum::BaseType::DEC),
      .num2 = BigNum("-123", BigNum::BaseType::DEC),
      .want_equal = false});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num1 = BigNum("1234", BigNum::BaseType::DEC),
      .num2 = BigNum("123", BigNum::BaseType::DEC),
      .want_equal = false});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .num1 = BigNum("124", BigNum::BaseType::DEC),
      .num2 = BigNum("123", BigNum::BaseType::DEC),
      .want_equal = false});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .num1 = BigNum("123", BigNum::BaseType::DEC),
      .num2 = BigNum("123", BigNum::BaseType::DEC),
      .want_equal = true});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .num1 = BigNum("123", BigNum::BaseType::HEX),
      .num2 = BigNum("123", BigNum::BaseType::DEC),
      .want_equal = false});
  test_cases.emplace_back(TestCase{
      .name = "case 8",
      .num1 = BigNum("7b", BigNum::BaseType::HEX),
      .num2 = BigNum("123", BigNum::BaseType::DEC),
      .want_equal = true});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    if (cur_test_case.want_equal) {
      EXPECT_TRUE(cur_test_case.num1 == cur_test_case.num2)
          << "Test " << cur_test_case.name << " failed, index " << ii;
      EXPECT_FALSE(cur_test_case.num1 != cur_test_case.num2)
          << "Test " << cur_test_case.name << " failed, index " << ii;
    } else {
      EXPECT_TRUE(cur_test_case.num1 != cur_test_case.num2)
          << "Test " << cur_test_case.name << " failed, index " << ii;
      EXPECT_FALSE(cur_test_case.num1 == cur_test_case.num2)
          << "Test " << cur_test_case.name << " failed, index " << ii;
    }
  }
}

TEST(BIG_NUM_TEST, Compare_test) {
  struct TestCase {
    std::string name;

    BigNum a;
    BigNum b;

    bool want_result_g;   // a>b
    bool want_result_ge;  // a>=b
    bool want_result_l;   // a<b
    bool want_result_le;  // a<=b
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = BigNum("0"),
      .b = BigNum("0"),
      .want_result_g = false,
      .want_result_ge = true,
      .want_result_l = false,
      .want_result_le = true});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = BigNum("1"),
      .b = BigNum("0"),
      .want_result_g = true,
      .want_result_ge = true,
      .want_result_l = false,
      .want_result_le = false});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .a = BigNum("-1"),
      .b = BigNum("0"),
      .want_result_g = false,
      .want_result_ge = false,
      .want_result_l = true,
      .want_result_le = true});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .a = BigNum("0"),
      .b = BigNum("1"),
      .want_result_g = false,
      .want_result_ge = false,
      .want_result_l = true,
      .want_result_le = true});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .a = BigNum("0"),
      .b = BigNum("-1"),
      .want_result_g = true,
      .want_result_ge = true,
      .want_result_l = false,
      .want_result_le = false});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .a = BigNum("123"),
      .b = BigNum("123"),
      .want_result_g = false,
      .want_result_ge = true,
      .want_result_l = false,
      .want_result_le = true});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .a = BigNum("123"),
      .b = BigNum("122"),
      .want_result_g = true,
      .want_result_ge = true,
      .want_result_l = false,
      .want_result_le = false});
  test_cases.emplace_back(TestCase{
      .name = "case 8",
      .a = BigNum("-123"),
      .b = BigNum("-12"),
      .want_result_g = false,
      .want_result_ge = false,
      .want_result_l = true,
      .want_result_le = true});
  test_cases.emplace_back(TestCase{
      .name = "case 9",
      .a = BigNum("-123"),
      .b = BigNum("122"),
      .want_result_g = false,
      .want_result_ge = false,
      .want_result_l = true,
      .want_result_le = true});
  test_cases.emplace_back(TestCase{
      .name = "case 10",
      .a = BigNum("122"),
      .b = BigNum("-123"),
      .want_result_g = true,
      .want_result_ge = true,
      .want_result_l = false,
      .want_result_le = false});
  test_cases.emplace_back(TestCase{
      .name = "case 11",
      .a = BigNum("255"),
      .b = BigNum("FE", BigNum::BaseType::HEX),
      .want_result_g = true,
      .want_result_ge = true,
      .want_result_l = false,
      .want_result_le = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    EXPECT_EQ(cur_test_case.a > cur_test_case.b, cur_test_case.want_result_g)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    EXPECT_EQ(cur_test_case.a >= cur_test_case.b, cur_test_case.want_result_ge)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    EXPECT_EQ(cur_test_case.a < cur_test_case.b, cur_test_case.want_result_l)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    EXPECT_EQ(cur_test_case.a <= cur_test_case.b, cur_test_case.want_result_le)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, Add_test) {
  struct TestCase {
    std::string name;

    BigNum a;
    BigNum b;

    BigNum want_result;  // want_result=a+b
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = BigNum("0"),
      .b = BigNum("0"),
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = BigNum("888"),
      .b = BigNum("11"),
      .want_result = BigNum("899")});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .a = BigNum("-11"),
      .b = BigNum("-888"),
      .want_result = BigNum("-899")});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .a = BigNum("999"),
      .b = BigNum("11"),
      .want_result = BigNum("1010")});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .a = BigNum(UINT32_MAX - 1, UINT32_MAX),
      .b = BigNum(3, UINT32_MAX),
      .want_result = BigNum(static_cast<int64_t>(UINT32_MAX) + 2, UINT32_MAX)});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .a = BigNum("0"),
      .b = BigNum("-0"),
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .a = BigNum("1234"),
      .b = BigNum("-11"),
      .want_result = BigNum("1223")});
  test_cases.emplace_back(TestCase{
      .name = "case 8",
      .a = BigNum("-1234"),
      .b = BigNum("11"),
      .want_result = BigNum("-1223")});
  test_cases.emplace_back(TestCase{
      .name = "case 9",
      .a = BigNum("-1234"),
      .b = BigNum("1234"),
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 10",
      .a = BigNum("-1234"),
      .b = BigNum("1233"),
      .want_result = BigNum("-1")});
  test_cases.emplace_back(TestCase{
      .name = "case 11",
      .a = BigNum("1234"),
      .b = BigNum("-235"),
      .want_result = BigNum("999")});
  test_cases.emplace_back(TestCase{
      .name = "case 12",
      .a = BigNum("1234"),
      .b = BigNum("-ABC", BigNum::BaseType::HEX),  // 2748
      .want_result = BigNum("-1514")});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    BigNum ret1 = cur_test_case.a + cur_test_case.b;
    EXPECT_EQ(ret1, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    BigNum ret2(cur_test_case.a);
    ret2 += cur_test_case.b;
    EXPECT_EQ(ret2, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    BigNum ret3 = cur_test_case.a - (-cur_test_case.b);
    EXPECT_EQ(ret3, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    BigNum ret4(cur_test_case.a);
    ret4 -= (-cur_test_case.b);
    EXPECT_EQ(ret4, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, DoubleAdd_test) {
  struct TestCase {
    std::string name;

    // a+1=b
    BigNum a;
    BigNum b;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = BigNum("0"),
      .b = BigNum("1")});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = BigNum("-1"),
      .b = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .a = BigNum("9"),
      .b = BigNum("10")});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    {
      BigNum ret1(cur_test_case.a);
      BigNum ret2 = ret1++;
      EXPECT_EQ(ret1, cur_test_case.b)
          << "Test " << cur_test_case.name << " failed, index " << ii;
      EXPECT_EQ(ret2, cur_test_case.a)
          << "Test " << cur_test_case.name << " failed, index " << ii;
    }
    {
      BigNum ret1(cur_test_case.a);
      BigNum ret2 = ++ret1;
      EXPECT_EQ(ret1, cur_test_case.b)
          << "Test " << cur_test_case.name << " failed, index " << ii;
      EXPECT_EQ(ret2, cur_test_case.b)
          << "Test " << cur_test_case.name << " failed, index " << ii;
    }
    {
      BigNum ret1(cur_test_case.b);
      BigNum ret2 = ret1--;
      EXPECT_EQ(ret1, cur_test_case.a)
          << "Test " << cur_test_case.name << " failed, index " << ii;
      EXPECT_EQ(ret2, cur_test_case.b)
          << "Test " << cur_test_case.name << " failed, index " << ii;
    }
    {
      BigNum ret1(cur_test_case.b);
      BigNum ret2 = --ret1;
      EXPECT_EQ(ret1, cur_test_case.a)
          << "Test " << cur_test_case.name << " failed, index " << ii;
      EXPECT_EQ(ret2, cur_test_case.a)
          << "Test " << cur_test_case.name << " failed, index " << ii;
    }
  }
}

TEST(BIG_NUM_TEST, Multiply_test) {
  struct TestCase {
    std::string name;

    BigNum a;
    BigNum b;

    BigNum want_result;  // want_result=a*b
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = BigNum("0"),
      .b = BigNum("0"),
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = BigNum("1"),
      .b = BigNum("0"),
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .a = BigNum("100"),
      .b = BigNum("200"),
      .want_result = BigNum("20000")});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .a = BigNum("-123"),
      .b = BigNum("456"),
      .want_result = BigNum("-56088")});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .a = BigNum("-15"),
      .b = BigNum("-20"),
      .want_result = BigNum("300")});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .a = BigNum("FF", BigNum::BaseType::HEX),  // 255
      .b = BigNum("24"),
      .want_result = BigNum("6120")});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .a = BigNum(UINT32_MAX - 1),
      .b = BigNum(UINT32_MAX - 1),
      .want_result = BigNum((UINT32_MAX - 1) / 2) * BigNum((UINT32_MAX - 1) / 2) * BigNum(4)});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    BigNum ret = cur_test_case.a * cur_test_case.b;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.a *= cur_test_case.b;
    EXPECT_EQ(cur_test_case.a, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, Divide_test) {
  struct TestCase {
    std::string name;

    BigNum dividend;
    BigNum divisor;

    BigNum want_result_quotient;
    BigNum want_result_remainder;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .dividend = BigNum("328"),
      .divisor = BigNum("36"),
      .want_result_quotient = BigNum("9"),
      .want_result_remainder = BigNum("4"),
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .dividend = BigNum("328"),
      .divisor = BigNum("0"),
      .want_result_quotient = BigNum("0"),
      .want_result_remainder = BigNum("0"),
      .want_exp = true});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .dividend = BigNum("1234"),
      .divisor = BigNum("1"),
      .want_result_quotient = BigNum("1234"),
      .want_result_remainder = BigNum("0"),
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .dividend = BigNum("1234"),
      .divisor = BigNum("-3"),
      .want_result_quotient = BigNum("-411"),
      .want_result_remainder = BigNum("1"),
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .dividend = BigNum("-1234"),
      .divisor = BigNum("5"),
      .want_result_quotient = BigNum("-246"),
      .want_result_remainder = BigNum("-4"),
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .dividend = BigNum("-1234"),
      .divisor = BigNum("-8"),
      .want_result_quotient = BigNum("154"),
      .want_result_remainder = BigNum("-2"),
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .dividend = BigNum("-99"),
      .divisor = BigNum("-100"),
      .want_result_quotient = BigNum("0"),
      .want_result_remainder = BigNum("-99"),
      .want_exp = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    bool get_exp = false;
    try {
      auto ret = cur_test_case.dividend.Div(cur_test_case.divisor);
      EXPECT_EQ(ret.first, cur_test_case.want_result_quotient)
          << "Test " << cur_test_case.name << " failed, index " << ii;
      EXPECT_EQ(ret.second, cur_test_case.want_result_remainder)
          << "Test " << cur_test_case.name << " failed, index " << ii;

      BigNum num1(cur_test_case.dividend);
      EXPECT_EQ(num1 / cur_test_case.divisor, cur_test_case.want_result_quotient)
          << "Test " << cur_test_case.name << " failed, index " << ii;

      num1 /= cur_test_case.divisor;
      EXPECT_EQ(num1, cur_test_case.want_result_quotient)
          << "Test " << cur_test_case.name << " failed, index " << ii;

      BigNum num2(cur_test_case.dividend);
      EXPECT_EQ(num2 % cur_test_case.divisor, cur_test_case.want_result_remainder)
          << "Test " << cur_test_case.name << " failed, index " << ii;

      num2 %= cur_test_case.divisor;
      EXPECT_EQ(num2, cur_test_case.want_result_remainder)
          << "Test " << cur_test_case.name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, cur_test_case.want_exp)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, LeftShift_test) {
  struct TestCase {
    std::string name;

    BigNum num;
    size_t n;

    BigNum want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = BigNum("0"),
      .n = 3,
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = BigNum("123", BigNum::BaseType::DEC, 0),
      .n = 3,
      .want_result = BigNum("123000")});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = BigNum("-123", BigNum::BaseType::DEC, 0),
      .n = 3,
      .want_result = BigNum("-123000")});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    BigNum ret = cur_test_case.num << cur_test_case.n;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.num <<= cur_test_case.n;
    EXPECT_EQ(cur_test_case.num, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, RightShift_test) {
  struct TestCase {
    std::string name;

    BigNum num;
    size_t n;

    BigNum want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = BigNum("0"),
      .n = 3,
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = BigNum("123", BigNum::BaseType::DEC, 0),
      .n = 2,
      .want_result = BigNum("1")});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = BigNum("123", BigNum::BaseType::DEC, 0),
      .n = 3,
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num = BigNum("123", BigNum::BaseType::DEC, 0),
      .n = 4,
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num = BigNum("-123", BigNum::BaseType::DEC, 0),
      .n = 1,
      .want_result = BigNum("-12")});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    BigNum ret = cur_test_case.num >> cur_test_case.n;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.num >>= cur_test_case.n;
    EXPECT_EQ(cur_test_case.num, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, ostream_test) {
  struct TestCase {
    std::string name;

    BigNum num;
    BigNum::BaseType type;

    std::string want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .num = BigNum("-0"),
      .type = BigNum::BaseType::DEC,
      .want_result = "0"});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = BigNum(123),
      .type = BigNum::BaseType::DEC,
      .want_result = "123"});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .num = BigNum(-123),
      .type = BigNum::BaseType::DEC,
      .want_result = "-123"});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .num = BigNum(123),
      .type = BigNum::BaseType::OCT,
      .want_result = "173"});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .num = BigNum(123),
      .type = BigNum::BaseType::HEX,
      .want_result = "7b"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    std::stringstream ss;
    switch (cur_test_case.type) {
      case BigNum::BaseType::OCT:
        ss << std::oct;
        break;
      case BigNum::BaseType::HEX:
        ss << std::hex;
        break;
      default:
        break;
    }
    ss << cur_test_case.num;
    EXPECT_STREQ(ss.str().c_str(), cur_test_case.want_result.c_str())
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, Pow_test) {
  struct TestCase {
    std::string name;

    BigNum value;
    uint32_t n;

    BigNum want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .value = BigNum("0"),
      .n = 0,
      .want_result = BigNum("1")});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .value = BigNum("2"),
      .n = 0,
      .want_result = BigNum("1")});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .value = BigNum("0"),
      .n = 10,
      .want_result = BigNum("0")});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .value = BigNum("2"),
      .n = 10,
      .want_result = BigNum("1024")});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .value = BigNum("-2"),
      .n = 10,
      .want_result = BigNum("1024")});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .value = BigNum("-2"),
      .n = 11,
      .want_result = BigNum("-2048")});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    auto ret = BigNum::Pow(cur_test_case.value, cur_test_case.n);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

}  // namespace ytlib
