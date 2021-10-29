/**
 * @file bool_expression.hpp
 * @author WT
 * @brief bool表达式解析工具
 * @note bool表达式解析工具
 * @date 2021-10-28
 */
#pragma once

#include <functional>
#include <map>
#include <queue>
#include <stack>
#include <string>

namespace ytlib {

/**
 * @brief Bool表达式解析工具
 * @note 支持“()|&!”运算符构成的bool表达式的值计算，
 */
class BoolExpCalc {
 public:
  using ExpKeyCheckFun = std::function<bool(std::string_view expression_key)>;
  using ExpKeyCalcFun = std::function<bool(std::string_view expression_key)>;

 public:
  explicit BoolExpCalc() {}
  explicit BoolExpCalc(ExpKeyCheckFun check_fun, ExpKeyCalcFun calc_fun) : exp_key_check_fun_(check_fun),
                                                                           exp_key_calc_fun_(calc_fun) {}
  virtual ~BoolExpCalc() {}

  /**
   * @brief 检查表达式合法性
   *
   * @param[in] expression 表达式
   * @return true 合法
   * @return false 不合法
   */
  bool CheckExp(std::string_view expression) const {
    enum class ExpState : uint8_t {
      BeginSymbol,     // 开始或(
      NegationSymbol,  // !
      AndOrSymbol,     // &和|
      RightBracket,    // )
      ExpKey,
    };

    int br_num = 0;  // 括号层数
    size_t cur_exp_key_begin_pos = 0;
    ExpState cur_state = ExpState::BeginSymbol;
    for (size_t ii = 0; ii < expression.size(); ++ii) {
      switch (cur_state) {
        case ExpState::BeginSymbol:
          if (expression[ii] == '(') {
            ++br_num;
          } else if (expression[ii] == '!') {
            cur_state = ExpState::NegationSymbol;
          } else if (expression[ii] == '&' || expression[ii] == '|' || expression[ii] == ')') {
            return false;
          } else {
            cur_state = ExpState::ExpKey;
            cur_exp_key_begin_pos = ii;
          }
          break;
        case ExpState::NegationSymbol:
        case ExpState::AndOrSymbol:
          if (expression[ii] == '(') {
            cur_state = ExpState::BeginSymbol;
            ++br_num;
          } else if (expression[ii] == '!') {
            cur_state = ExpState::NegationSymbol;
          } else if (expression[ii] == '&' || expression[ii] == '|' || expression[ii] == ')') {
            return false;
          } else {
            cur_state = ExpState::ExpKey;
            cur_exp_key_begin_pos = ii;
          }
          break;
        case ExpState::RightBracket:
          if (expression[ii] == '&' || expression[ii] == '|') {
            cur_state = ExpState::AndOrSymbol;
          } else if (expression[ii] == ')') {
            if ((--br_num) < 0) return false;
          } else {
            return false;
          }
          break;
        case ExpState::ExpKey:
          if (expression[ii] == '(' || expression[ii] == '!') {
            return false;
          } else if (expression[ii] == '&' || expression[ii] == '|') {
            if (exp_key_check_fun_ && !exp_key_check_fun_(expression.substr(cur_exp_key_begin_pos, ii - cur_exp_key_begin_pos)))
              return false;
            cur_state = ExpState::AndOrSymbol;
          } else if (expression[ii] == ')') {
            if ((--br_num) < 0) return false;
            if (exp_key_check_fun_ && !exp_key_check_fun_(expression.substr(cur_exp_key_begin_pos, ii - cur_exp_key_begin_pos)))
              return false;
            cur_state = ExpState::RightBracket;
          }
          break;
        default:
          return false;
      }
    }
    if (cur_state != ExpState::ExpKey && cur_state != ExpState::RightBracket) return false;
    if (br_num != 0) return false;
    return true;
  }

  /**
   * @brief 计算表达式值
   * @note 如果表达式非法会抛异常
   * @param[in] expression 表达式
   * @return true
   * @return false
   */
  bool CalcExp(std::string_view expression) const {
    return true;
  }

  /**
   * @brief 默认单表达式合法性检查函数
   *
   * @param[in] expression_key 待检查单表达式，不为空
   * @return true 合法
   * @return false 不合法
   */
  static bool DefaultExpKeyCheckFun(std::string_view expression_key) {
    return true;
  }

  /**
   * @brief 默认单表达式值计算函数
   *
   * @param[in] expression_key 待检查单表达式，其中不可能含有“()|&!”且不为空
   * @return true
   * @return false
   */
  static bool DefaultExpKeyCalcFun(std::string_view expression_key) {
    if (expression_key == "F" || expression_key == "False") return false;
    return true;
  }

  // 符号优先级
  static uint8_t Priority(char c) {
    switch (c) {
      case '(':
        return 1;
      case '|':
      case '&':
        return 2;
      case '!':
        return 3;
      case ')':
        return 4;
      default:
        return 0;
    }
  }

  /**
   * @brief 转为后缀表达式
   *
   * @param[in] expression 表达式
   * @return std::queue<std::string> 后缀表达式
   */
  static std::queue<std::string> ConvertToSuffixExp(std::string_view expression) {
    std::stack<char> st;  // 辅助栈
    st.push('#');
    std::queue<std::string> qu;  // 存后缀表达式

    for (size_t ii = 0; ii < expression.size(); ++ii) {
      if (expression[ii] == '!' || expression[ii] == '&' || expression[ii] == '|' ||
          expression[ii] == '(' || expression[ii] == ')') {
        if (expression[ii] == '(') {
          st.push(expression[ii]);
        } else if (expression[ii] == ')') {
          while (st.top() != '(') {
            qu.push(std::string(1, st.top()));
            st.pop();
          }
          st.pop();
        } else if (Priority(expression[ii]) >= Priority(st.top())) {
          st.push(expression[ii]);
        } else {
          while (Priority(expression[ii]) <= Priority(st.top())) {
            qu.push(std::string(1, st.top()));
            st.pop();
          }
          st.push(expression[ii]);
        }
      } else {
        size_t pos0 = ii;
        for (++ii; ii < expression.length(); ++ii) {
          if (expression[ii] == '!' || expression[ii] == '&' || expression[ii] == '|' ||
              expression[ii] == '(' || expression[ii] == ')') break;
        }
        qu.push(std::string(expression.substr(pos0, ii - pos0)));
        --ii;
      }
    }
    while (st.top() != '#') {
      qu.push(std::string(1, st.top()));
      st.pop();
    }

    return qu;
  }

 protected:
  ExpKeyCheckFun exp_key_check_fun_ = DefaultExpKeyCheckFun;  /// 单表达式合法性检查函数
  ExpKeyCalcFun exp_key_calc_fun_ = DefaultExpKeyCalcFun;     /// 单表达式值计算函数
};

}  // namespace ytlib
