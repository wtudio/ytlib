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

    BoolExpCalc calc;
    std::string expression;
    bool want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .calc = BoolExpCalc(),
      .expression = "abc",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .calc = BoolExpCalc(),
      .expression = "(a&b|!c)",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .calc = BoolExpCalc(),
      .expression = "((a&b)|(!c))",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .calc = BoolExpCalc(),
      .expression = "!!!!a",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .calc = BoolExpCalc(check_fun, BoolExpCalc::DefaultExpKeyCalcFun),
      .expression = "!(testkey)",
      .want_result = true});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .calc = BoolExpCalc(check_fun, BoolExpCalc::DefaultExpKeyCalcFun),
      .expression = "!(abc)",
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .calc = BoolExpCalc(),
      .expression = "",
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "bad case 2",
      .calc = BoolExpCalc(),
      .expression = "&",
      .want_result = false});
  test_cases.emplace_back(TestCase{
      .name = "bad case 3",
      .calc = BoolExpCalc(),
      .expression = "(a))",
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(test_cases[ii].calc.CheckExp(test_cases[ii].expression), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BOOL_EXP_TEST, ConvertToSuffixExp_test) {
  struct TestCase {
    std::string name;

    std::string expression;
    std::vector<std::string> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .expression = "",
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .expression = "(abc)",
      .want_result = {"abc"}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .expression = "(!abc&def)",
      .want_result = {"abc", "!", "def", "&"}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .expression = "!a|(b&c)",
      .want_result = {"a", "!", "b", "c", "&", "|"}});
  test_cases.emplace_back(TestCase{
      .name = "bad case 1",
      .expression = "())",
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "bad case 2",
      .expression = "()abc)",
      .want_result = {"abc"}});
  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::queue<std::string> result = BoolExpCalc::ConvertToSuffixExp(test_cases[ii].expression);
    std::vector<std::string> result_vec;
    while (!result.empty()) {
      result_vec.push_back(result.front());
      result.pop();
    }
    EXPECT_EQ(result_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BOOL_EXP_TEST, CalcExp_test) {
  struct TestCase {
    std::string name;

    BoolExpCalc calc;
    std::string expression;
    bool want_result;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .calc = BoolExpCalc(),
      .expression = "",
      .want_result = true,
      .want_exp = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    bool get_exp = false;
    try {
      EXPECT_EQ(test_cases[ii].calc.CalcExp(test_cases[ii].expression), test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
