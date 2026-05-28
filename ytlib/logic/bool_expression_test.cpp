#include <gtest/gtest.h>

#include "bool_expression.hpp"

namespace ytlib {

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
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 5",
        .calc = BoolExpCalculator(),
        .expression = "!(testkey)",
        .want_result = true});
    cur_case.calc.SetKeyCheckFun(check_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 6",
        .calc = BoolExpCalculator(),
        .expression = "!(abc)",
        .want_result = false});
    cur_case.calc.SetKeyCheckFun(check_fun);
  }
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
      .name = "bad case 1",
      .calc = BoolExpCalculator(),
      .expression = "",
      .want_mid_result = {},
      .want_exp = true});
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .calc = BoolExpCalculator(),
      .expression = "((abc))",
      .want_mid_result = {"abc"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .calc = BoolExpCalculator(),
      .expression = "(!abc&def)",
      .want_mid_result = {"abc", "!", "def", "&"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .calc = BoolExpCalculator(),
      .expression = "(!aa|(bb&(cc|dd)))&ee",
      .want_mid_result = {"aa", "!", "bb", "cc", "dd", "|", "&", "|", "ee", "&"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .calc = BoolExpCalculator(),
      .expression = "T&F|(!T)",
      .want_mid_result = {"T", "F", "&", "T", "!", "|"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 5: double negation",
      .calc = BoolExpCalculator(),
      .expression = "!!a",
      .want_mid_result = {"a", "!", "!"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 6: quadruple negation",
      .calc = BoolExpCalculator(),
      .expression = "!!!!a",
      .want_mid_result = {"a", "!", "!", "!", "!"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 7: double negation with parens",
      .calc = BoolExpCalculator(),
      .expression = "!!(a)",
      .want_mid_result = {"a", "!", "!"},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 8: double negation mixed with &",
      .calc = BoolExpCalculator(),
      .expression = "!!a&b",
      .want_mid_result = {"a", "!", "!", "b", "&"},
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
  auto calc_fun = [](std::string_view expression_key) {
    if (expression_key == "F" || expression_key == "False") return false;
    return true;
  };

  struct TestCase {
    std::string name;

    BoolExpCalculator calc;
    std::string expression;
    bool want_result;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 1",
        .calc = BoolExpCalculator(),
        .expression = "T",
        .want_result = true,
        .want_exp = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 2",
        .calc = BoolExpCalculator(),
        .expression = "F",
        .want_result = false,
        .want_exp = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 3",
        .calc = BoolExpCalculator(),
        .expression = "T&F|(!T)",
        .want_result = false,
        .want_exp = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 4",
        .calc = BoolExpCalculator(),
        .expression = "T&!(T&F|(!T))",
        .want_result = true,
        .want_exp = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 5: double negation true",
        .calc = BoolExpCalculator(),
        .expression = "!!T",
        .want_result = true,
        .want_exp = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 6: double negation false",
        .calc = BoolExpCalculator(),
        .expression = "!!F",
        .want_result = false,
        .want_exp = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 7: quadruple negation",
        .calc = BoolExpCalculator(),
        .expression = "!!!!T",
        .want_result = true,
        .want_exp = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "bad case 1",
        .calc = BoolExpCalculator(),
        .expression = "",
        .want_result = true,
        .want_exp = true});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  test_cases.emplace_back(TestCase{
      .name = "bad case 2: missing calc fun",
      .calc = BoolExpCalculator(),
      .expression = "T",
      .want_result = false,
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

TEST(BOOL_EXP_TEST, ShortCircuit_test) {
  // 验证&/|按源码从左到右求值, 且短路时跳过的key不会被key_calc_fun_调用
  std::vector<std::string> evaluated;
  auto calc_fun = [&evaluated](std::string_view expression_key) {
    evaluated.emplace_back(expression_key);
    if (expression_key == "F" || expression_key == "False") return false;
    return true;
  };

  struct TestCase {
    std::string name;

    BoolExpCalculator calc;
    std::string expression;
    bool want_result;
    std::vector<std::string> want_evaluated;
  };
  std::vector<TestCase> test_cases;
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 1: F&right short-circuit",
        .calc = BoolExpCalculator(),
        .expression = "F&right",
        .want_result = false,
        .want_evaluated = {"F"}});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 2: T|right short-circuit",
        .calc = BoolExpCalculator(),
        .expression = "T|right",
        .want_result = true,
        .want_evaluated = {"T"}});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 3: T&right both evaluated",
        .calc = BoolExpCalculator(),
        .expression = "T&right",
        .want_result = true,
        .want_evaluated = {"T", "right"}});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 4: F|right both evaluated",
        .calc = BoolExpCalculator(),
        .expression = "F|right",
        .want_result = true,
        .want_evaluated = {"F", "right"}});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 5: a&b&c left-to-right order",
        .calc = BoolExpCalculator(),
        .expression = "a&b&c",
        .want_result = true,
        .want_evaluated = {"a", "b", "c"}});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    evaluated.clear();
    EXPECT_EQ(test_cases[ii].calc.Calc(test_cases[ii].expression), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(evaluated, test_cases[ii].want_evaluated)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(BOOL_EXP_TEST, TwoStepCalc_test) {
  // 验证文档中提到的PreCalc -> Calc(MidResultClass)两步调用
  auto calc_fun = [](std::string_view expression_key) {
    if (expression_key == "F" || expression_key == "False") return false;
    return true;
  };

  struct TestCase {
    std::string name;

    BoolExpCalculator calc;
    std::string expression;
    bool want_result;
  };
  std::vector<TestCase> test_cases;
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 1",
        .calc = BoolExpCalculator(),
        .expression = "T",
        .want_result = true});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 2",
        .calc = BoolExpCalculator(),
        .expression = "T&F",
        .want_result = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 3",
        .calc = BoolExpCalculator(),
        .expression = "T&!(T&F|(!T))",
        .want_result = true});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto mid = test_cases[ii].calc.PreCalc(test_cases[ii].expression);
    // 同一中间结果多次调用应得到相同结果
    EXPECT_EQ(test_cases[ii].calc.Calc(mid), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].calc.Calc(mid), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed (second call), index " << ii;
  }
}

TEST(BOOL_EXP_TEST, MultiCharKey_test) {
  // 验证多字符key与运算符混合的场景
  auto calc_fun = [](std::string_view expression_key) {
    return expression_key == "true_key" || expression_key == "another_true";
  };

  struct TestCase {
    std::string name;

    BoolExpCalculator calc;
    std::string expression;
    bool want_result;
  };
  std::vector<TestCase> test_cases;
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 1: single true key",
        .calc = BoolExpCalculator(),
        .expression = "true_key",
        .want_result = true});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 2: single false key",
        .calc = BoolExpCalculator(),
        .expression = "false_key",
        .want_result = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 3: two true keys with &",
        .calc = BoolExpCalculator(),
        .expression = "true_key&another_true",
        .want_result = true});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 4: true & false",
        .calc = BoolExpCalculator(),
        .expression = "true_key&false_key",
        .want_result = false});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 5: false | true",
        .calc = BoolExpCalculator(),
        .expression = "false_key|true_key",
        .want_result = true});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }
  {
    auto& cur_case = test_cases.emplace_back(TestCase{
        .name = "case 6: !false_key & true_key",
        .calc = BoolExpCalculator(),
        .expression = "!false_key&true_key",
        .want_result = true});
    cur_case.calc.SetKeyCalcFun(calc_fun);
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(test_cases[ii].calc.Calc(test_cases[ii].expression), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
