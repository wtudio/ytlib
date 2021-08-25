/**
 * @file big_num.hpp
 * @brief 大数
 * @note 可转换进制的大数类
 * @author WT
 * @date 2019-07-26
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <iostream>
#include <vector>

namespace ytlib {
/**
 * @brief 大整数工具
 * 
 */

class BigNum {
 public:
  /**
   * @brief 典型进制
   * 
   */
  enum class BaseType : uint8_t {
    BIN = 2,
    OCT = 8,
    DEC = 10,
    HEX = 16,
  };

 public:
  BigNum() { content_.emplace_back(0); }

  /**
   * @brief 从一个int64_t构建
   * @note 使用默认进制为10。最终值为num*(base^order)
   * @param num 有效数字部分
   * @param base 进制。不可小于2，否则按默认值10处理
   * @param order 进制乘方
   */
  explicit BigNum(int64_t num, uint32_t base = 10, uint32_t order = 0) {
    base_ = (base < 2) ? 10 : base;

    if (num < 0) {
      symbol_ = false;
      num = -num;
    }

    if (num != 0) {
      while (order--) content_.push_back(0);
    }

    do {
      content_.push_back(num % base_);
      num /= base_;
    } while (num);
  }

  /**
   * @brief 从一个string构建
   * @note 使用默认进制为16
   * @param str 
   * @param base 进制
   */
  explicit BigNum(const std::string& str, BaseType base = BaseType::DEC) {
    base_ = static_cast<uint32_t>(base);

    if (str.empty()) {
      content_.emplace_back(0);
      return;
    }

    size_t pos = 0;
    if (str[0] == '-') {
      symbol_ = false;
      pos = 1;
    } else if (str[0] == '+') {
      pos = 1;
    }

    while (pos < str.size()) {
      uint32_t curNum = base_;
      if (str[pos] >= '0' && str[pos] <= '9')
        curNum = (str[pos] - '0');
      else if (str[pos] >= 'A' && str[pos] <= 'F')
        curNum = (str[pos] - 'A') + 10;
      else if (str[pos] >= 'a' && str[pos] <= 'f')
        curNum = (str[pos] - 'a') + 10;

      if (curNum >= base_) break;

      content_.emplace_back(curNum);
      ++pos;
    }

    if (content_.empty()) {
      content_.emplace_back(0);
    } else {
      std::reverse(content_.begin(), content_.end());
    }
  }

  ~BigNum() {}

  void ReBase(uint32_t base) {
    if (base < 2) return;
    if (base_ == base) return;
  }

  void Clear() {
    symbol_ = true;
    content_.clear();
    content_.emplace_back(0);
  }

  bool Empty() const {
    return ((content_.size() == 1) && (content_[0] == 0));
  }

  operator bool() const {
    return !Empty();
  }

  bool operator==(const BigNum& value) const {
    if (base_ != value.base_) return false;

    if (Empty() && value.Empty()) return true;
    if (symbol_ != value.symbol_) return false;
    if (content_.size() != value.content_.size()) return false;

    const size_t& len = content_.size();
    for (size_t ii = 0; ii < len; ++ii) {
      if (content_[ii] != value.content_[ii]) return false;
    }

    return true;
  }
  bool operator!=(const BigNum& value) const {
    return !(*this == value);
  }

  BigNum operator+(const BigNum& value) const {
    //需要确保进制相同
    //assert(base_ == value.base_);
    const BigNum *pNum1 = this, *pNum2 = &value;
    size_t len1 = pNum1->content_.size(), len2 = pNum2->content_.size();
    BigNum re(0, base_);
    if (symbol_ ^ value.symbol_) {
      //异符号相加，用绝对值大的减小的，符号与大的相同。默认num1的绝对值大
      if (len1 == len2) {
        //从高位开始判断
        for (size_t ii = len1 - 1; ii > 0; --ii) {
          if (pNum1->content_[ii] > pNum2->content_[ii])
            break;
          else if (pNum1->content_[ii] < pNum2->content_[ii]) {
            std::swap(len1, len2);
            std::swap(pNum1, pNum2);
            break;
          }
        }
      } else if (len1 < len2) {
        std::swap(len1, len2);
        std::swap(pNum1, pNum2);
      }
      re.symbol_ = pNum1->symbol_;
      bool flag = false;  //借位标志
      for (size_t ii = 0; ii < len2; ++ii) {
        //需要借位的情况
        if (flag && pNum1->content_[ii] == 0) {
          flag = true;
          re.content_[ii] = base_ - 1 - pNum2->content_[ii];
        } else if ((pNum1->content_[ii] - (flag ? 1 : 0)) < pNum2->content_[ii]) {
          re.content_[ii] = base_ - pNum2->content_[ii] + (pNum1->content_[ii] - (flag ? 1 : 0));
          flag = true;
        } else {
          flag = false;
          re.content_[ii] = pNum1->content_[ii] - pNum2->content_[ii];
        }
        re.content_.push_back(0);
      }
      for (size_t ii = len2; ii < len1; ++ii) {
        if (flag && pNum1->content_[ii] == 0) {
          flag = true;
          re.content_[ii] = base_ - 1;
        } else {
          flag = false;
          re.content_[ii] = pNum1->content_[ii] - 1;
        }
        re.content_.push_back(0);
      }
    } else {
      //同符号相加
      re.symbol_ = symbol_;
      //被加数num1的位数较大
      if (len1 < len2) {
        std::swap(len1, len2);
        std::swap(pNum1, pNum2);
      }
      //从低位开始加
      for (size_t ii = 0; ii < len2; ++ii) {
        uint32_t tmp = pNum1->content_[ii] + pNum2->content_[ii] + re.content_[ii];
        if ((base_ && tmp >= base_) || tmp < pNum1->content_[ii] || (tmp == pNum1->content_[ii] && re.content_[ii] == 1)) {
          re.content_.push_back(1);
          tmp -= base_;
        } else
          re.content_.push_back(0);
        re.content_[ii] = tmp;
      }
      for (size_t ii = len2; ii < len1; ++ii) {
        if (pNum1->content_[ii] == (base_ - 1) && re.content_[ii] == 1) {
          re.content_[ii] = 0;
          re.content_.push_back(1);
        } else {
          re.content_[ii] += pNum1->content_[ii];
          re.content_.push_back(0);
        }
      }
    }
    //去除最后端的0
    while (re.content_.size() > 1 && re.content_[re.content_.size() - 1] == 0) re.content_.pop_back();
    return re;
  }
  BigNum& operator+=(const BigNum& value) {
    (*this) = operator+(value);
    return *this;
  }

  ///++i
  BigNum& operator++() {
    operator+=(BigNum(1, base_));
    return *this;
  }
  ///i++
  const BigNum operator++(int) {
    BigNum re(*this);
    operator+=(BigNum(1, base_));
    return re;
  }

  BigNum operator-(const BigNum& value) const {
    return operator+(-value);
  }
  BigNum& operator-=(const BigNum& value) {
    (*this) = operator+(-value);
    return *this;
  }

  ///--i
  BigNum& operator--() {
    operator+=(BigNum(-1, base_));
    return *this;
  }
  ///i--
  const BigNum operator--(int) {
    BigNum re(*this);
    operator+=(BigNum(-1, base_));
    return re;
  }

  BigNum operator-() const {
    BigNum re(*this);
    re.symbol_ = !re.symbol_;
    return re;
  }

  BigNum operator*(const BigNum& value) const {
    //assert(base_ == value.base_);
    BigNum re(0, base_);
    size_t len1 = this->content_.size(), len2 = value.content_.size();
    for (size_t ii = 0; ii < len1; ++ii) {
      if (this->content_[ii] != 0) {
        for (size_t jj = 0; jj < len2; ++jj) {
          if (value.content_[jj] != 0)
            re += BigNum(int64_t(this->content_[ii]) * value.content_[jj], base_, static_cast<uint32_t>(ii + jj));
        }
      }
    }
    re.symbol_ = !(this->symbol_ ^ value.symbol_);
    while (re.content_.size() > 1 && re.content_[re.content_.size() - 1] == 0) re.content_.pop_back();
    return re;
  }
  BigNum& operator*=(const BigNum& value) {
    (*this) = operator*(value);
    return *this;
  }

  BigNum operator/(const BigNum& value) const {
    std::pair<BigNum, BigNum> re = div(value);
    return re.first;
  }
  BigNum& operator/=(const BigNum& value) {
    (*this) = operator/(value);
    return (*this);
  }

  BigNum operator%(const BigNum& value) const {
    std::pair<BigNum, BigNum> re = div(value);
    return re.second;
  }
  BigNum& operator%=(const BigNum& value) {
    (*this) = operator%(value);
    return (*this);
  }

  BigNum operator<<(size_t n) const {
    BigNum re = *this;
    re.content_.insert(re.content_.begin(), 0);
    return re;
  }
  BigNum& operator<<=(size_t n) {
    content_.insert(content_.begin(), 0);
    return *this;
  }
  BigNum operator>>(size_t n) const {
    BigNum re = *this;
    if (re.content_.size()) re.content_.erase(re.content_.begin());
    return re;
  }
  BigNum& operator>>=(size_t n) {
    if (content_.size()) content_.erase(content_.begin());
    return *this;
  }

  bool operator<(const BigNum& value) const {
    //assert(base_ == value.base_);
    if (BigNum::operator bool() && value) return false;
    if (!symbol_ && value.symbol_) return true;
    if (symbol_ && !(value.symbol_)) return false;
    if (content_.size() != value.content_.size()) return symbol_ ^ (content_.size() > value.content_.size());
    size_t len = content_.size();
    //从高位开始判断
    for (size_t ii = len - 1; ii > 0; --ii) {
      if (content_[ii] == value.content_[ii]) continue;
      if (symbol_ ^ (content_[ii] > value.content_[ii])) return true;
      return false;
    }
    return symbol_ ^ (content_[0] >= value.content_[0]);
  }
  bool operator>(const BigNum& value) const {
    return value < (*this);
  }
  bool operator<=(const BigNum& value) const {
    return BigNum::operator<(value) || BigNum::operator==(value);
  }
  bool operator>=(const BigNum& value) const {
    return BigNum::operator>(value) || BigNum::operator==(value);
  }

  BigNum& Swap(BigNum& value) {
    return *this;
  }

  /**
   * @brief 除，同时获取商和余数
   * 
   * @param val 除数
   * @return std::pair<BigNum, BigNum> <商, 余数>
   */
  std::pair<BigNum, BigNum> div(const BigNum& val) const {
    // assert(base_ == val.base_ && val);

    return std::make_pair(BigNum(), BigNum());
  }

  friend std::ostream& operator<<(std::ostream& out, const BigNum& val) {
    //先输出进制
    out << '[' << val.base_ << ']';
    //符号位
    if (!val.symbol_ && val) out << '-';

    if (val.base_ > 1 && val.base_ <= 16) {
      //如果进制在16之内则采用16进制的符号
      size_t len = val.content_.size();
      out << std::hex;
      for (size_t ii = len - 1; ii < len; --ii) {
        out << val.content_[ii];
      }
      out << std::dec;
    } else {
      //否则需要在各个位之间空开一格，以十进制输出数据
      size_t len = val.content_.size();
      for (size_t ii = len - 1; ii < len; --ii) {
        out << val.content_[ii] << ' ';
      }
    }
    return out;
  }

  bool get_symbol() const { return symbol_; }
  const std::vector<uint32_t>& get_content() const { return content_; }
  uint32_t get_base() const { return base_; }

  static BigNum Abs(const BigNum& val) {
    BigNum re(val);
    re.symbol_ = true;
    return re;
  }

  static BigNum Pow(const BigNum& num, uint32_t n) {
    return BigNum();
  }

 private:
  bool symbol_ = true;             //正负
  std::vector<uint32_t> content_;  //采用大端存储，越高位在越后面，方便增加位数
  uint32_t base_ = 10;             //进制，不能小于2
};

BigNum abs(const BigNum& value) { return BigNum::Abs(value); }

void swap(BigNum& a, BigNum& b) { a.Swap(b); }

}  // namespace ytlib
