#include <gtest/gtest.h>

#include "bool_expression.hpp"
#include "rule_tool.hpp"

namespace ytlib {

TEST(RULE_TOOL_TEST, RunRules) {
  int rule_result = 0;

  std::map<std::string, std::function<bool()> > rule_map;
  rule_map.emplace("r1", [&]() {
    rule_result = 1;
    return false;
  });
  rule_map.emplace("r2", [&]() {
    rule_result = 2;
    return true;
  });
  rule_map.emplace("r3", [&]() {
    rule_result = 3;
    return true;
  });

  std::string hit_rule = RunRules(std::vector<std::string>{"r1", "r2", "r3"}, rule_map);
  EXPECT_STREQ(hit_rule.c_str(), "r2");
  EXPECT_EQ(rule_result, 2);
}

TEST(BOOL_EXP_TEST, CheckExp_test) {
  auto check_fun = [](std::string_view expression_key) {
    if (expression_key == "testkey") return true;
    return false;
  };

  struct TestCase {
    std::string name;

    BoolExpCalculator calc;
    std::string expression;
    bool want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .calc = BoolExpCalculator(),
      .expression = "abc",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .calc = BoolExpCalculator(),
      .expression = "(a&b|!c)",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .calc = BoolExpCalculator(),
      .expression = "((a&b)|(!c))",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .calc = BoolExpCalculator(),
      .expression = "!!!!a",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .calc = BoolExpCalculator(check_fun, BoolExpCalculator::DefaultKeyCalcFun),
      .expression = "!(testkey)",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .calc = BoolExpCalculator(check_fun, BoolExpCalculator::DefaultKeyCalcFun),
      .expression = "!(abc)",
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .calc = BoolExpCalculator(),
      .expression = "T&!(T&F|(!T))",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .calc = BoolExpCalculator(),
      .expression = "",
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "bad case 2",
      .calc = BoolExpCalculator(),
      .expression = "&",
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "bad case 3",
      .calc = BoolExpCalculator(),
      .expression = "(a))",
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(test_cases[ii].calc.CheckExp(test_cases[ii].expression), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BOOL_EXP_TEST, PreCalc_test) {
  struct TestCase {
    std::string name;

    BoolExpCalculator calc;
    std::string expression;
    std::vector<std::string> want_mid_result;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .calc = BoolExpCalculator(),
      .expression = "",
      .want_mid_result = {},
      .want_exp = true});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .calc = BoolExpCalculator(),
      .expression = "((abc))",
      .want_mid_result = {"abc"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .calc = BoolExpCalculator(),
      .expression = "(!abc&def)",
      .want_mid_result = {"abc", "!", "def", "&"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .calc = BoolExpCalculator(),
      .expression = "(!aa|(bb&(cc|dd)))&ee",
      .want_mid_result = {"aa", "!", "bb", "cc", "dd", "|", "&", "|", "ee", "&"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .calc = BoolExpCalculator(),
      .expression = "T&F|(!T)",
      .want_mid_result = {"T", "F", "&", "T", "!", "|"},
      .want_exp = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    bool get_exp = false;
    try {
      EXPECT_EQ(test_cases[ii].calc.PreCalc(test_cases[ii].expression), test_cases[ii].want_mid_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BOOL_EXP_TEST, Calc_test) {
  struct TestCase {
    std::string name;

    BoolExpCalculator calc;
    std::string expression;
    bool want_result;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .calc = BoolExpCalculator(),
      .expression = "T",
      .want_result = true,
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .calc = BoolExpCalculator(),
      .expression = "F",
      .want_result = false,
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .calc = BoolExpCalculator(),
      .expression = "T&F|(!T)",
      .want_result = false,
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .calc = BoolExpCalculator(),
      .expression = "T&!(T&F|(!T))",
      .want_result = true,
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .calc = BoolExpCalculator(),
      .expression = "",
      .want_result = true,
      .want_exp = true});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    bool get_exp = false;
    try {
      EXPECT_EQ(test_cases[ii].calc.Calc(test_cases[ii].expression), test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
