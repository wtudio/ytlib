/**
 * @file math_tools.hpp
 * @brief 基础数学库
 * @details 一些常用的数学工具，包括判断素数、求最大公约数/最小公倍数、分解质因数、排列组合数等
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <cinttypes>
#include <cmath>
#include <map>
#include <vector>

#include "math_tools_exports.h"

namespace ytlib {

/**
 * @brief 计算有多少个1
 * @details 计算uint64_t的二进制形式中有多少个1
 * @param n 输入的数字
 * @return n的二进制形式中有多少个1
 */
MATH_TOOLS_API uint8_t CountOne(uint64_t n);

//计算有多少个0
inline uint8_t CountZero(uint64_t n) {
  return CountOne(~n);
}

//计算在pos之前的第一个1的位置
MATH_TOOLS_API uint8_t FindFirstOne(uint64_t val, uint8_t pos);

//判断是否是质数
MATH_TOOLS_API bool IsPrime(const uint64_t& num);

//求最大公约数
MATH_TOOLS_API uint64_t Gcd(uint64_t num1, uint64_t num2);

//求最小公倍数：num1*num2/gcd(num1,num2)
inline uint64_t Lcm(uint64_t num1, uint64_t num2) {
  return num1 / Gcd(num1, num2) * num2;
}

//分解质因数
MATH_TOOLS_API void Factoring(uint64_t num, std::vector<uint64_t>& re);

MATH_TOOLS_API void Factoring(uint64_t num, std::map<uint64_t, uint64_t>& re);

//累乘，需保证n>=m，且m!=0
MATH_TOOLS_API uint64_t Mul(uint64_t n, uint64_t m = 1);

//排列数，从从n个不同元素中，任取m(m≤n,m与n均为自然数）个元素，其排列的个数。A(n,m)=n!/(n-m)!
MATH_TOOLS_API uint64_t Arn(uint64_t n, uint64_t m);

//排列数递归求解，已知A(n,mr)求A(n,m)
MATH_TOOLS_API uint64_t Arn(uint64_t n, uint64_t m, uint64_t a, uint64_t mr);

//组合数，C(n,m)=A(n,m)/m!=n!/(m!*(n-m)!)。C(n,m)=C(n,n-m)
MATH_TOOLS_API uint64_t Crn(uint64_t n, uint64_t m);

//组合数递归求解
MATH_TOOLS_API uint64_t Crn(uint64_t n, uint64_t m, uint64_t c, uint64_t mr);

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