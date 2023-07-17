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
 * @note 考虑运算效率的话，最好使用UINT32_MAX作为进制数，这样普通运算都会退化成uint32_t数值运算
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
   * @note 使用默认进制为UINT32_MAX。最终值为num*(base^order)
   * @param[in] num 有效数字部分
   * @param[in] base 进制。不可小于2，否则按默认值UINT32_MAX处理
   * @param[in] order 进制乘方
   */
  explicit BigNum(int64_t num, uint32_t base = UINT32_MAX, size_t order = 0) {
    base_ = (base < 2) ? UINT32_MAX : base;

    if (num < 0) {
      symbol_ = false;
      num = -num;
    }

    if (num != 0) content_.resize(order);

    do {
      lldiv_t tmp = lldiv(num, base_);
      content_.emplace_back(static_cast<uint32_t>(tmp.rem));
      num = tmp.quot;
    } while (num);
  }

  /**
   * @brief 从一个string构建
   * @note 使用默认进制为16
   * @param[in] str 如 "+123","-fff"
   * @param[in] str_base 输入的字符串的进制
   * @param[in] base 实际运算时的进制
   */
  explicit BigNum(const std::string& str, BaseType str_base = BaseType::DEC, uint32_t base = UINT32_MAX) {
    if (str.empty()) {
      content_.emplace_back(0);
      base_ = (base < 2) ? static_cast<uint32_t>(str_base) : base;
      return;
    }

    base_ = static_cast<uint32_t>(str_base);

    size_t pos = 0;
    if (str[0] == '-') {
      symbol_ = false;
      pos = 1;
    } else if (str[0] == '+') {
      pos = 1;
    }

    content_.reserve(str.size() - pos);

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

    while (content_.size() > 1 && content_.back() == 0) content_.pop_back();

    ReBase(base);
  }

  ~BigNum() = default;

  /**
   * @brief 从一个uint64_t赋值
   * @note 使用默认进制为UINT32_MAX。最终值为num*(base^order)
   * @param[in] num 有效数字部分
   * @param[in] symbol 符号
   * @param[in] base 进制。不可小于2，否则按默认值UINT32_MAX处理
   * @param[in] order 进制乘方
   * @return BigNum 结果
   */
  static BigNum AssignU64(uint64_t num, bool symbol = true, uint32_t base = UINT32_MAX, size_t order = 0) {
    BigNum ret;
    ret.base_ = (base < 2) ? UINT32_MAX : base;
    ret.symbol_ = symbol;

    if (num != 0) ret.content_.resize(order);

    do {
      ret.content_.emplace_back(static_cast<uint32_t>(num % ret.base_));
      num /= ret.base_;
    } while (num);

    return ret;
  }

  /**
   * @brief 获取当前使用的进制
   *
   * @return uint32_t 当前使用的进制
   */
  uint32_t GetBase() const { return base_; }

  /**
   * @brief 重新设定进制
   *
   * @param[in] base 新进制
   */
  void ReBase(uint32_t base) {
    if (base < 2) return;
    if (base_ == base) return;

    std::vector<uint32_t> ret;

    while (!content_.empty()) {
      uint64_t cur_num = 0;
      for (size_t ii = content_.size() - 1; ii < content_.size(); --ii) {
        cur_num = cur_num * base_ + content_[ii];
        content_[ii] = static_cast<uint32_t>(cur_num / base);
        cur_num %= base;
      }
      ret.emplace_back(static_cast<uint32_t>(cur_num));
      while (content_.size() > 0 && content_.back() == 0) content_.pop_back();
    }

    base_ = base;
    while (ret.size() > 1 && ret.back() == 0) ret.pop_back();
    content_ = std::move(ret);
  }

  /**
   * @brief 清零
   *
   */
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
    const bool& empty_flag1 = Empty();
    const bool& empty_flag2 = value.Empty();
    if (empty_flag1 && empty_flag2) return true;
    if (empty_flag1 || empty_flag2) return false;

    if (symbol_ != value.symbol_) return false;

    const BigNum& real_val = (base_ == value.base_) ? value : ChangeBase(value, base_);

    if (content_.size() != real_val.content_.size()) return false;

    const size_t& len = content_.size();
    for (size_t ii = 0; ii < len; ++ii) {
      if (content_[ii] != real_val.content_[ii]) return false;
    }

    return true;
  }
  bool operator!=(const BigNum& value) const {
    return !(*this == value);
  }

  bool operator<(const BigNum& value) const {
    return (Compare(*this, value) < 0);
  }
  bool operator>(const BigNum& value) const {
    return (Compare(*this, value) > 0);
  }
  bool operator<=(const BigNum& value) const {
    return (Compare(*this, value) <= 0);
  }
  bool operator>=(const BigNum& value) const {
    return (Compare(*this, value) >= 0);
  }

  BigNum operator+(const BigNum& value) const {
    const BigNum& real_val = (base_ == value.base_) ? value : ChangeBase(value, base_);

    const BigNum *num1_ptr = this, *num2_ptr = &real_val;
    size_t len1 = num1_ptr->content_.size(), len2 = num2_ptr->content_.size();
    BigNum re(0, base_);
    if (symbol_ ^ real_val.symbol_) {
      // 异符号相加，用绝对值大的减小的，符号与大的相同。默认num1的绝对值大于等于num2绝对值
      if (len1 == len2) {
        // 从高位开始判断
        for (size_t ii = len1 - 1; ii < len1; --ii) {
          if (num1_ptr->content_[ii] > num2_ptr->content_[ii])
            break;
          if (num1_ptr->content_[ii] < num2_ptr->content_[ii]) {
            std::swap(len1, len2);
            std::swap(num1_ptr, num2_ptr);
            break;
          }
        }
      } else if (len1 < len2) {
        std::swap(len1, len2);
        std::swap(num1_ptr, num2_ptr);
      }
      re.symbol_ = num1_ptr->symbol_;
      re.content_.resize(len1);
      uint32_t flag = 0;  // 借位标志
      for (size_t ii = 0; ii < len2; ++ii) {
        // 需要借位的情况
        if (flag && num1_ptr->content_[ii] == 0) {
          re.content_[ii] = base_ - 1 - num2_ptr->content_[ii];
          flag = 1;
        } else if ((num1_ptr->content_[ii] - flag) < num2_ptr->content_[ii]) {
          re.content_[ii] = base_ - num2_ptr->content_[ii] + (num1_ptr->content_[ii] - flag);
          flag = 1;
        } else {
          re.content_[ii] = num1_ptr->content_[ii] - flag - num2_ptr->content_[ii];
          flag = 0;
        }
      }
      for (size_t ii = len2; ii < len1; ++ii) {
        if (flag && num1_ptr->content_[ii] == 0) {
          re.content_[ii] = base_ - 1;
          flag = 1;
        } else {
          re.content_[ii] = num1_ptr->content_[ii] - flag;
          flag = 0;
        }
      }
    } else {
      // 同符号相加，被加数num1的位数大于等于num2的。实际大小不要求num1>=num2
      if (len1 < len2) {
        std::swap(len1, len2);
        std::swap(num1_ptr, num2_ptr);
      }
      re.symbol_ = symbol_;
      re.content_.resize(len1 + 1);
      // 从低位开始加
      for (size_t ii = 0; ii < len2; ++ii) {
        re.content_[ii] += (num1_ptr->content_[ii] + num2_ptr->content_[ii]);
        if (re.content_[ii] >= base_ || re.content_[ii] < num1_ptr->content_[ii]) {
          re.content_[ii + 1] = 1;
          re.content_[ii] -= base_;
        }
      }
      for (size_t ii = len2; ii < len1; ++ii) {
        re.content_[ii] += num1_ptr->content_[ii];
        if (re.content_[ii] == base_) {
          re.content_[ii] = 0;
          re.content_[ii + 1] = 1;
        }
      }
    }
    // 去除最后端的0
    while (re.content_.size() > 1 && re.content_.back() == 0) re.content_.pop_back();
    return re;
  }
  BigNum& operator+=(const BigNum& value) {
    (*this) = operator+(value);
    return *this;
  }

  ///++i
  BigNum& operator++() {
    (*this) = operator+(BigNum(1, base_));
    return *this;
  }
  /// i++
  const BigNum operator++(int) {
    BigNum re(*this);
    (*this) = operator+(BigNum(1, base_));
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
    (*this) = operator+(BigNum(-1, base_));
    return *this;
  }
  /// i--
  const BigNum operator--(int) {
    BigNum re(*this);
    (*this) = operator+(BigNum(-1, base_));
    return re;
  }

  BigNum operator-() const {
    BigNum re(*this);
    re.symbol_ = !re.symbol_;
    return re;
  }

  BigNum operator*(const BigNum& value) const {
    const BigNum& real_val = (base_ == value.base_) ? value : ChangeBase(value, base_);

    BigNum re(0, base_);
    const size_t& len1 = content_.size();
    const size_t& len2 = real_val.content_.size();
    for (size_t ii = 0; ii < len1; ++ii) {
      if (content_[ii] == 0) continue;
      for (size_t jj = 0; jj < len2; ++jj) {
        if (real_val.content_[jj] == 0) continue;
        re += BigNum::AssignU64(static_cast<uint64_t>(content_[ii]) * real_val.content_[jj], true, base_, ii + jj);
      }
    }
    re.symbol_ = !(symbol_ ^ real_val.symbol_);
    while (re.content_.size() > 1 && re.content_.back() == 0) re.content_.pop_back();
    return re;
  }
  BigNum& operator*=(const BigNum& value) {
    (*this) = operator*(value);
    return *this;
  }

  /**
   * @brief 除，同时获取商和余数
   * @note 被除数-余数 == 除数*商，余数符号始终与被除数相同
   * @param value 除数
   * @return std::pair<BigNum, BigNum> <商, 余数>
   */
  std::pair<BigNum, BigNum> Div(const BigNum& value) const {
    if (value.Empty())
      throw std::invalid_argument("Divisor cannot be 0.");

    const BigNum& divisor = Abs((base_ == value.base_) ? value : ChangeBase(value, base_));

    if (content_.size() < divisor.content_.size())
      return std::make_pair(BigNum(0, base_), *this);

    BigNum remainder = Abs(*this);

    size_t quotient_len = content_.size() - divisor.content_.size() + 1;
    std::vector<uint32_t> quotient_vec(quotient_len);

    const uint32_t& divisor_head = divisor.content_.back();  // 除数当前最高位

    for (size_t ii = quotient_len - 1; ii < quotient_len; --ii) {
      uint64_t cur_num = 0;  // 被除数当前最高位/最高两位
      size_t cur_remainder_ii = ii + divisor.content_.size() - 1;
      if (cur_remainder_ii + 1 < remainder.content_.size()) {
        cur_num = remainder.content_[cur_remainder_ii] + (remainder.content_[cur_remainder_ii + 1] * base_);
      } else if (cur_remainder_ii < remainder.content_.size()) {
        cur_num = remainder.content_[cur_remainder_ii];
      }

      // 确定k上下界[a1/(b1+1)+1, (a1+1)/b1]
      uint32_t min_k = static_cast<uint32_t>(cur_num / (divisor_head + 1));
      uint32_t max_k = static_cast<uint32_t>((cur_num + 1) / divisor_head);

      const BigNum& cur_divisor = (divisor << ii);

      while (min_k <= max_k) {
        uint32_t cur_k = min_k + (max_k - min_k) / 2;
        BigNum cur_remainder = remainder - BigNum(static_cast<int64_t>(cur_k), base_, ii) * divisor;
        if (cur_remainder.symbol_ == false) {
          max_k = cur_k - 1;
        } else if (cur_remainder >= cur_divisor) {
          min_k = cur_k + 1;
        } else {
          quotient_vec[ii] = cur_k;
          remainder = std::move(cur_remainder);
          break;
        }
      }
    }

    while (quotient_vec.size() > 1 && quotient_vec.back() == 0) quotient_vec.pop_back();

    BigNum quotient(0, base_);
    quotient.content_ = std::move(quotient_vec);
    quotient.symbol_ = !(symbol_ ^ value.symbol_);

    remainder.symbol_ = symbol_;

    return std::make_pair(std::move(quotient), std::move(remainder));
  }

  BigNum operator/(const BigNum& value) const {
    return Div(value).first;
  }
  BigNum& operator/=(const BigNum& value) {
    (*this) = Div(value).first;
    return (*this);
  }

  BigNum operator%(const BigNum& value) const {
    return Div(value).second;
  }
  BigNum& operator%=(const BigNum& value) {
    (*this) = Div(value).second;
    return (*this);
  }

  BigNum operator<<(size_t n) const {
    BigNum re(*this);
    if (re.Empty()) return re;

    re.content_.insert(re.content_.begin(), n, 0);
    return re;
  }
  BigNum& operator<<=(size_t n) {
    if (Empty()) return *this;
    content_.insert(content_.begin(), n, 0);
    return *this;
  }
  BigNum operator>>(size_t n) const {
    BigNum re(*this);
    if (n >= re.content_.size()) {
      re.content_.clear();
      re.content_.emplace_back(0);
    } else {
      re.content_.erase(re.content_.begin(), re.content_.begin() + n);
    }
    return re;
  }
  BigNum& operator>>=(size_t n) {
    if (n >= content_.size()) {
      content_.clear();
      content_.emplace_back(0);
    } else {
      content_.erase(content_.begin(), content_.begin() + n);
    }
    return *this;
  }

  /**
   * @brief 打印
   * @note 支持8、10、16进制打印，c++不支持直接的二进制打印
   * @param out
   * @param val
   * @return std::ostream&
   */
  friend std::ostream& operator<<(std::ostream& out, const BigNum& val) {
    // 符号位
    if (!val.symbol_ && !val.Empty()) out << '-';

    const BigNum* real_val_ptr = &val;
    BigNum tmp;
    if ((out.flags() & std::ostream::oct) && (val.base_ != static_cast<uint32_t>(BaseType::OCT))) {
      tmp = val;
      tmp.ReBase(static_cast<uint32_t>(BaseType::OCT));
      real_val_ptr = &tmp;
    } else if ((out.flags() & std::ostream::hex) && (val.base_ != static_cast<uint32_t>(BaseType::HEX))) {
      tmp = val;
      tmp.ReBase(static_cast<uint32_t>(BaseType::HEX));
      real_val_ptr = &tmp;
    } else if (val.base_ != static_cast<uint32_t>(BaseType::DEC)) {
      // 默认用十进制输出
      tmp = val;
      tmp.ReBase(static_cast<uint32_t>(BaseType::DEC));
      real_val_ptr = &tmp;
    }

    size_t len = real_val_ptr->content_.size();
    for (size_t ii = len - 1; ii < len; --ii) {
      out << real_val_ptr->content_[ii];
    }

    return out;
  }

  bool Symbol() const { return symbol_; }
  const std::vector<uint32_t>& Content() const { return content_; }
  uint32_t Base() const { return base_; }

  /**
   * @brief 比较大小
   *
   * @param val_l
   * @param val_r
   * @return int32_t 1:val_l>val_r; 0:val_l==val_r; -1:val_l<val_r
   */
  static int32_t Compare(const BigNum& val_l, const BigNum& val_r) {
    if (val_l.Empty() && val_r.Empty()) return 0;
    if (val_l.symbol_ && !val_r.symbol_) return 1;
    if (!val_l.symbol_ && val_r.symbol_) return -1;

    const BigNum& real_val_r = (val_l.base_ == val_r.base_) ? val_r : ChangeBase(val_r, val_l.base_);

    if (val_l.content_.size() != real_val_r.content_.size())
      return ((val_l.symbol_ ^ (val_l.content_.size() > real_val_r.content_.size())) ? -1 : 1);

    // 从高位开始判断
    const size_t& len = val_l.content_.size();
    for (size_t ii = len - 1; ii < len; --ii) {
      if (val_l.content_[ii] == real_val_r.content_[ii]) continue;
      return ((val_l.symbol_ ^ (val_l.content_[ii] > real_val_r.content_[ii])) ? -1 : 1);
    }

    return 0;
  }

  static BigNum Abs(const BigNum& val) {
    BigNum re(val);
    re.symbol_ = true;
    return re;
  }

  static BigNum Pow(const BigNum& num, uint32_t n) {
    BigNum re(1, num.base_);
    BigNum tmp(num);
    for (; n; n >>= 1) {
      if (n & 1)
        re *= tmp;
      tmp *= tmp;
    }
    return re;
  }

  static BigNum ChangeBase(const BigNum& num, uint32_t base) {
    BigNum ret(num);
    ret.ReBase(base);
    return ret;
  }

 private:
  bool symbol_ = true;             ///< 正负
  std::vector<uint32_t> content_;  ///< 采用大端存储，越高位在越后面，方便增加位数
  uint32_t base_ = UINT32_MAX;     ///< 进制，不能小于2
};

inline BigNum abs(const BigNum& value) { return BigNum::Abs(value); }

}  // namespace ytlib
