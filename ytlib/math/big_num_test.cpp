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
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .num = BigNum(0, 0, 0),
      .want_symbol = true,
      .want_content = {0},
      .want_base = 10});
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
      .num = BigNum("10110", BigNum::BaseType::BIN),
      .want_symbol = true,
      .want_content = {0, 1, 1, 0, 1},
      .want_base = 2});
  test_cases.emplace_back(TestCase{
      .name = "case 8",
      .num = BigNum("-12345", BigNum::BaseType::OCT),
      .want_symbol = false,
      .want_content = {5, 4, 3, 2, 1},
      .want_base = 8});
  test_cases.emplace_back(TestCase{
      .name = "case 9",
      .num = BigNum("987"),
      .want_symbol = true,
      .want_content = {7, 8, 9},
      .want_base = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 10",
      .num = BigNum("-ABc9", BigNum::BaseType::HEX),
      .want_symbol = false,
      .want_content = {9, 12, 11, 10},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 11",
      .num = BigNum("", BigNum::BaseType::HEX),
      .want_symbol = true,
      .want_content = {0},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 12",
      .num = BigNum("-", BigNum::BaseType::HEX),
      .want_symbol = false,
      .want_content = {0},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 13",
      .num = BigNum("-kkk", BigNum::BaseType::HEX),
      .want_symbol = false,
      .want_content = {0},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 14",
      .num = BigNum("-9abc@123", BigNum::BaseType::HEX),
      .want_symbol = false,
      .want_content = {12, 11, 10, 9},
      .want_base = 16});
  test_cases.emplace_back(TestCase{
      .name = "case 15",
      .num = BigNum("+9abc@123", BigNum::BaseType::DEC),
      .want_symbol = true,
      .want_content = {9},
      .want_base = 10});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(test_cases[ii].num.get_symbol(), test_cases[ii].want_symbol)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].num.get_content(), test_cases[ii].want_content)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].num.get_base(), test_cases[ii].want_base)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
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

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    test_cases[ii].num.ReBase(test_cases[ii].base);
    EXPECT_EQ(test_cases[ii].num.get_symbol(), test_cases[ii].want_symbol)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].num.get_content(), test_cases[ii].want_content)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].num.get_base(), test_cases[ii].want_base)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
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
      .num1 = BigNum("", BigNum::BaseType::HEX),
      .num2 = BigNum("", BigNum::BaseType::DEC),
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

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    if (test_cases[ii].want_equal) {
      EXPECT_TRUE(test_cases[ii].num1 == test_cases[ii].num2)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_FALSE(test_cases[ii].num1 != test_cases[ii].num2)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } else {
      EXPECT_TRUE(test_cases[ii].num1 != test_cases[ii].num2)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_FALSE(test_cases[ii].num1 == test_cases[ii].num2)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    }
  }
}

TEST(BIG_NUM_TEST, Add_test) {
  struct TestCase {
    std::string name;

    BigNum a;
    BigNum b;

    BigNum want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = BigNum("0"),
      .b = BigNum("0"),
      .want_result = BigNum("0")});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].a + test_cases[ii].b;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].a += test_cases[ii].b;
    EXPECT_EQ(test_cases[ii].a, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BIG_NUM_TEST, Sub_test) {
}

TEST(BIG_NUM_TEST, Multiply_test) {
}

TEST(BIG_NUM_TEST, Divide_test) {
}

TEST(BIG_NUM_TEST, Shift_test) {
}

TEST(BIG_NUM_TEST, Compare_test) {
}

TEST(BIG_NUM_TEST, ostream_test) {
}

TEST(BIG_NUM_TEST, Pow_test) {
}

}  // namespace ytlib
