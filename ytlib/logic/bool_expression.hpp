/**
 * @file bool_expression.hpp
 * @author WT
 * @brief bool表达式
 * @note bool表达式解析计算工具，支持“()|&!”运算符构成的bool表达式的值计算
 * @date 2021-10-28
 */
#pragma once

#include <functional>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace ytlib {

/**
 * @brief bool表达式计算器
 * @note 支持“()|&!”运算符构成的bool表达式的值计算
 * 使用前需要设定表达式key的校验和计算方法
 * 提供两种调用方式：
 * 1、直接调用 Calc(exp_str) 方法进行计算得到结果
 * 2、先调用 PreCalc(exp_str) 方法得到中间结果mid_result，再调用 Calc(mid_result) 进行计算。
 * 此方法适用于单表达式key计算结果会变化，且需要多次计算的场景。
 */
class BoolExpCalculator {
 public:
  using KeyCheckFun = std::function<bool(std::string_view key)>;  // 单表达式key校验函数接口
  using KeyCalcFun = std::function<bool(std::string_view key)>;   // 单表达式key计算函数接口
  using MidResultClass = std::vector<std::string>;                // 中间结果类型

 public:
  BoolExpCalculator() {}
  ~BoolExpCalculator() {}

  void SetKeyCheckFun(const KeyCheckFun& check_fun) {
    key_check_fun_ = check_fun;
  }

  void SetKeyCalcFun(const KeyCalcFun& calc_fun) {
    key_calc_fun_ = calc_fun;
  }

  /**
   * @brief 由bool表达式字符串形式计算表达式值
   * @note 如果表达式本身不合法会抛异常。如果表达式key计算函数内部抛异常也会抛出
   * @param[in] expression bool表达式字符串形式
   * @return true 表达式结果为true
   * @return false 表达式结果为false
   */
  bool Calc(std::string_view expression) const {
    return Calc(PreCalc(expression));
  }

  /**
   * @brief 预计算
   * @note 转为后缀表达式。如果表达式本身不合法会抛异常。
   * @param[in] expression bool表达式字符串形式
   * @return MidResultClass 预计算后的中间结果
   */
  MidResultClass PreCalc(std::string_view expression) const {
    if (!CheckExp(expression))
      throw std::logic_error("expression invalid.");

    std::stack<char> st;  // 辅助栈
    st.push('#');
    std::vector<std::string> qu;  // 存后缀表达式

    for (size_t ii = 0; ii < expression.size(); ++ii) {
      const char& cur_smb = expression[ii];
      if (cur_smb == '(') {
        st.push('(');
      } else if (cur_smb == ')') {
        while (st.top() != '(') {
          qu.push_back(std::string(1, st.top()));
          st.pop();
        }
        st.pop();
      } else if (cur_smb == '!' | cur_smb == '&' | cur_smb == '|') {
        if (Priority(cur_smb) > Priority(st.top())) {
          st.push(cur_smb);
        } else {
          while (Priority(cur_smb) <= Priority(st.top())) {
            qu.push_back(std::string(1, st.top()));
            st.pop();
          }
          st.push(cur_smb);
        }
      } else {
        size_t pos0 = ii;
        for (++ii; ii < expression.size(); ++ii) {
          if (expression[ii] == '!' || expression[ii] == '&' || expression[ii] == '|' ||
              expression[ii] == '(' || expression[ii] == ')') break;
        }
        qu.push_back(std::string(expression.substr(pos0, ii - pos0)));
        --ii;
      }
    }

    while (st.top() != '#') {
      qu.push_back(std::string(1, st.top()));
      st.pop();
    }

    return qu;
  }

  /**
   * @brief 由bool表达式中间结果计算表达式值
   * @note 如果表达式本身不合法会抛异常。如果表达式key计算函数内部抛异常也会抛出
   * @param[in] expression bool表达式后缀表达式形式
   * @return true 表达式结果为true
   * @return false 表达式结果为false
   */
  bool Calc(const MidResultClass& expression) const {
    struct Ret {
      Ret(std::string_view s) : has_ret(false), ret(false), exp_key(s) {}
      Ret(bool b) : has_ret(true), ret(b), exp_key() {}

      const bool has_ret;
      const bool ret;
      const std::string_view exp_key;
    };
    std::stack<Ret> st;  // 辅助栈

    // 用下标+vector模拟queue
    for (size_t pos = 0; pos < expression.size(); ++pos) {
      if (expression[pos] == "!") {
        const auto& last_ret = st.top();
        const bool cur_ret = last_ret.has_ret ? last_ret.ret : key_calc_fun_(last_ret.exp_key);
        st.pop();
        st.push(Ret(!cur_ret));
      } else if (expression[pos] == "&") {
        const auto& last_ret_1 = st.top();
        const bool cur_ret_1 = last_ret_1.has_ret ? last_ret_1.ret : key_calc_fun_(last_ret_1.exp_key);
        st.pop();
        if (!cur_ret_1) {
          st.pop();
          st.push(Ret(false));
        } else {
          const auto& last_ret_2 = st.top();
          const bool cur_ret_2 = last_ret_2.has_ret ? last_ret_2.ret : key_calc_fun_(last_ret_2.exp_key);
          st.pop();
          st.push(Ret(cur_ret_2));
        }
      } else if (expression[pos] == "|") {
        const auto& last_ret_1 = st.top();
        const bool cur_ret_1 = last_ret_1.has_ret ? last_ret_1.ret : key_calc_fun_(last_ret_1.exp_key);
        st.pop();
        if (cur_ret_1) {
          st.pop();
          st.push(Ret(true));
        } else {
          const auto& last_ret_2 = st.top();
          const bool cur_ret_2 = last_ret_2.has_ret ? last_ret_2.ret : key_calc_fun_(last_ret_2.exp_key);
          st.pop();
          st.push(Ret(cur_ret_2));
        }
      } else {
        st.push(Ret(expression[pos]));
      }
    }

    const auto& final_ret = st.top();
    return final_ret.has_ret ? final_ret.ret : key_calc_fun_(final_ret.exp_key);
  }

  /**
   * @brief 检查表达式合法性
   *
   * @param expression
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
    size_t cur_key_begin_pos = 0;
    ExpState cur_state = ExpState::BeginSymbol;
    for (size_t ii = 0; ii < expression.size(); ++ii) {
      const char& cur_smb = expression[ii];
      switch (cur_state) {
        case ExpState::BeginSymbol:
          if (cur_smb == '(') {
            ++br_num;
          } else if (cur_smb == '!') {
            cur_state = ExpState::NegationSymbol;
          } else if (cur_smb == '&' || cur_smb == '|' || cur_smb == ')') {
            return false;
          } else {
            cur_state = ExpState::ExpKey;
            cur_key_begin_pos = ii;
          }
          break;
        case ExpState::NegationSymbol:
        case ExpState::AndOrSymbol:
          if (cur_smb == '(') {
            cur_state = ExpState::BeginSymbol;
            ++br_num;
          } else if (cur_smb == '!') {
            cur_state = ExpState::NegationSymbol;
          } else if (cur_smb == '&' || cur_smb == '|' || cur_smb == ')') {
            return false;
          } else {
            cur_state = ExpState::ExpKey;
            cur_key_begin_pos = ii;
          }
          break;
        case ExpState::RightBracket:
          if (cur_smb == '&' || cur_smb == '|') {
            cur_state = ExpState::AndOrSymbol;
          } else if (cur_smb == ')') {
            if ((--br_num) < 0) return false;
          } else {
            return false;
          }
          break;
        case ExpState::ExpKey:
          if (cur_smb == '(' || cur_smb == '!') {
            return false;
          } else if (cur_smb == '&' || cur_smb == '|') {
            if (key_check_fun_ && !key_check_fun_(expression.substr(cur_key_begin_pos, ii - cur_key_begin_pos)))
              return false;
            cur_state = ExpState::AndOrSymbol;
          } else if (cur_smb == ')') {
            if ((--br_num) < 0) return false;
            if (key_check_fun_ && !key_check_fun_(expression.substr(cur_key_begin_pos, ii - cur_key_begin_pos)))
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
   * @brief 默认单表达式合法性检查函数
   *
   * @param[in] expression_key 待检查单表达式，不为空
   * @return true 合法
   * @return false 不合法
   */
  static bool DefaultKeyCheckFun(std::string_view expression_key) {
    return true;
  }

  /**
   * @brief 默认单表达式值计算函数
   *
   * @param[in] expression_key 待检查单表达式，其中不可能含有“()|&!”且不为空
   * @return true
   * @return false
   */
  static bool DefaultKeyCalcFun(std::string_view expression_key) {
    if (expression_key == "F" || expression_key == "False") return false;
    return true;
  }

 private:
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

 private:
  KeyCheckFun key_check_fun_ = DefaultKeyCheckFun;  // 单表达式合法性检查函数
  KeyCalcFun key_calc_fun_ = DefaultKeyCalcFun;     // 单表达式值计算函数
};

}  // namespace ytlib
