/**
 * @file Mathbase.h
 * @brief 基础数学库
 * @details 一些常用的数学工具，包括判断素数、求最大公约数/最小公倍数、分解质因数、排列组合数等
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <cassert>
#include <cmath>
#include <map>
#include <vector>

#if defined(USE_DOUBLE_PRECISION)
typedef double tfloat;  // double precision
#else
typedef float tfloat;  // single precision
#endif

#define PI 3.1415926535897932384626433832795028841971

//判断浮点数是否相等
#ifndef fequal
#define fequal(a, b) (((a - b) > -1e-6) && ((a - b) < 1e-6))
#endif

//一些工具
namespace ytlib {
//计算有多少个1
inline uint32_t count_1_(uint64_t n) {
  uint32_t num = 0;
  while (n) {
    n &= (n - 1);
    ++num;
  }
  return num;
}
//计算有多少个0
inline uint32_t count_0_(uint64_t n) {
  return count_1_(~n);
}

//计算在pos之前的第一个1的位置
inline uint32_t find_1_(uint64_t val, uint32_t pos) {
  assert(pos < 64);
  if (!val) return 64;
  while (0 == (val & ((uint64_t)1 << pos))) {
    ++pos;
  }
  return pos;
}

//判断是否是质数
inline bool isPrime(uint64_t num) {
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
inline uint64_t gcd(uint64_t num1, uint64_t num2) {
  if (num1 < num2) std::swap(num1, num2);
  if (num2 == 0) return num1;
  if (num1 & 1) {
    if (num2 & 1) return gcd(num2, num1 - num2);
    return gcd(num1, num2 >> 1);
  } else {
    if (num2 & 1) return gcd(num1 >> 1, num2);
    return (gcd(num1 >> 1, num2 >> 1) << 1);
  }
}
//求最小公倍数：num1*num2/gcd(num1,num2)
inline uint64_t lcm(uint64_t num1, uint64_t num2) {
  return num1 / gcd(num1, num2) * num2;
}
//分解质因数
inline void factoring(uint64_t num, std::vector<uint64_t>& re) {
  if (num < 2) return;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      re.push_back(ii);
      num /= ii;
    }
  }
}
inline void factoring(uint64_t num, std::map<uint64_t, uint64_t>& re) {
  if (num < 2) return;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      std::pair<std::map<uint64_t, uint64_t>::iterator, bool> tmp = re.insert(std::pair<uint64_t, uint64_t>(ii, 1));
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
//排列数递归求解，已知A(n,m_)求A(n,m)
inline uint64_t Arn(uint64_t n, uint64_t m, uint64_t A, uint64_t m_) {
  assert(n >= m && m && n >= m_ && m_);
  if (m < m_)
    return A / Mul(n - m + 2, n - m_ + 1);
  else if (m > m_)
    return A * Mul(n - m_ + 2, n - m + 1);
  else
    return A;
}

//组合数，C(n,m)=A(n,m)/m!=n!/(m!*(n-m)!)。C(n,m)=C(n,n-m)
inline uint64_t Crn(uint64_t n, uint64_t m) {
  assert(n >= m && m);
  if (n >= (2 * m + 1)) m = n - m;
  return (n == m) ? 1 : (Mul(n, m + 1) / Mul(n - m));
}
//组合数递归求解
inline uint64_t Crn(uint64_t n, uint64_t m, uint64_t C, uint64_t m_) {
  assert(n >= m && m && n >= m_ && m_);
  if (n >= (2 * m + 1)) m = n - m;
  if (m < m_)
    return C * Mul(m_, m + 1) / Mul(n - m, n - m_ + 1);
  else if (m > m_)
    return C * Mul(n - m_, n - m + 1) / Mul(m, m_ + 1);
  else
    return C;
}

//等差数列求和
inline tfloat SumAP(tfloat a1, tfloat d, uint64_t n) {
  return n * (a1 + d / 2 * (n - 1));
}

//等比数列求和
inline tfloat SumGP(tfloat a1, tfloat q, uint64_t n) {
  assert(q != 0.0);
  return (q == 1.0) ? (n * a1) : (a1 * (1 - std::pow(q, n)) / (1 - q));
}

// pow
inline uint64_t pow(uint64_t value, uint64_t n) {
  uint64_t re = 1;
  for (; n; n >>= 1) {
    if (n & 1)
      re *= value;
    value *= value;
  }
  return re;
}

}  // namespace ytlib