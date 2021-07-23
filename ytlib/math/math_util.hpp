/**
 * @file math_tools.hpp
 * @brief 基础数学库
 * @details 一些常用的数学工具，包括判断素数、求最大公约数/最小公倍数、分解质因数、排列组合数等
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <map>
#include <vector>

namespace ytlib {

/**
 * @brief 计算有多少个1
 * @details 计算uint64_t的二进制形式中有多少个1
 * @param n 输入的数字
 * @return n的二进制形式中有多少个1
 */
inline uint8_t CountOne(uint64_t n) {
  uint8_t num = 0;
  while (n) {
    n &= (n - 1);
    ++num;
  }
  return num;
}

//计算有多少个0
inline uint8_t CountZero(uint64_t n) {
  return CountOne(~n);
}

//计算在pos之前的第一个1的位置
inline uint8_t FindFirstOne(uint64_t val, uint8_t pos) {
  assert(pos < 64);
  if (!val) return 64;
  while (0 == (val & ((uint64_t)1 << pos)))
    ++pos;

  return pos;
}

//判断是否是质数
inline bool IsPrime(const uint64_t& num) {
  //两个较小数另外处理
  if (num == 2 || num == 3) return true;
  //不在6的倍数两侧的一定不是质数
  if (num % 6 != 1 && num % 6 != 5) return false;
  uint64_t tmp = uint64_t(std::sqrt(num));
  //在6的倍数两侧的也可能不是质数
  for (uint64_t i = 5; i <= tmp; i += 6)
    if (num % i == 0 || num % (i + 2) == 0)
      return false;
  //排除所有，剩余的是质数
  return true;
}

//求最大公约数
inline uint64_t Gcd(uint64_t num1, uint64_t num2) {
  if (num1 < num2) std::swap(num1, num2);
  if (num2 == 0) return num1;
  if (num1 & 1) {
    if (num2 & 1) return Gcd(num2, num1 - num2);
    return Gcd(num1, num2 >> 1);
  } else {
    if (num2 & 1) return Gcd(num1 >> 1, num2);
    return (Gcd(num1 >> 1, num2 >> 1) << 1);
  }
}

//求最小公倍数：num1*num2/gcd(num1,num2)
inline uint64_t Lcm(uint64_t num1, uint64_t num2) {
  return num1 / Gcd(num1, num2) * num2;
}

//分解质因数
inline void Factoring(uint64_t num, std::vector<uint64_t>& re) {
  if (num < 2) return;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      re.emplace_back(ii);
      num /= ii;
    }
  }
}

inline void Factoring(uint64_t num, std::map<uint64_t, uint64_t>& re) {
  if (num < 2) return;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      std::pair<std::map<uint64_t, uint64_t>::iterator, bool> tmp = re.emplace(ii, 1);
      if (!tmp.second) ++(tmp.first->second);
      num /= ii;
    }
  }
}

//累乘，需保证n>=m，且m!=0
inline uint64_t Mul(uint64_t n, uint64_t m = 1) {
  assert(n >= m && m);
  uint64_t re = n;
  while ((--n) >= m) re *= n;
  return re;
}

//排列数，从从n个不同元素中，任取m(m≤n,m与n均为自然数）个元素，其排列的个数。A(n,m)=n!/(n-m)!
inline uint64_t Arn(uint64_t n, uint64_t m) {
  assert(n >= m && m);
  return Mul(n, n - m + 1);
}

//排列数递归求解，已知A(n,mr)求A(n,m)
inline uint64_t Arn(uint64_t n, uint64_t m, uint64_t a, uint64_t mr) {
  assert(n >= m && m && n >= mr && mr);
  if (m < mr)
    return a / Mul(n - m + 2, n - mr + 1);
  else if (m > mr)
    return a * Mul(n - mr + 2, n - m + 1);
  else
    return a;
}

//组合数，C(n,m)=A(n,m)/m!=n!/(m!*(n-m)!)。C(n,m)=C(n,n-m)
inline uint64_t Crn(uint64_t n, uint64_t m) {
  assert(n >= m && m);
  if (n >= (2 * m + 1)) m = n - m;
  return (n == m) ? 1 : (Mul(n, m + 1) / Mul(n - m));
}

//组合数递归求解
inline uint64_t Crn(uint64_t n, uint64_t m, uint64_t c, uint64_t mr) {
  assert(n >= m && m && n >= mr && mr);
  if (n >= (2 * m + 1)) m = n - m;
  if (m < mr)
    return c * Mul(mr, m + 1) / Mul(n - m, n - mr + 1);
  else if (m > mr)
    return c * Mul(n - mr, n - m + 1) / Mul(m, mr + 1);
  else
    return c;
}

//等差数列求和
template <typename T>
T SumAP(const T& a1, const T& d, uint64_t n) {
  return n * (a1 + d / 2 * (n - 1));
}

//等比数列求和
template <typename T>
T SumGP(const T& a1, const T& q, uint64_t n) {
  assert(q != 0.0);
  return (q == 1.0) ? (n * a1) : (a1 * (1 - std::pow(q, n)) / (1 - q));
}

// pow
template <typename T>
T Pow(const T& value, uint64_t n) {
  T re = 1;
  for (; n; n >>= 1) {
    if (n & 1)
      re *= value;
    value *= value;
  }
  return re;
}

}  // namespace ytlib