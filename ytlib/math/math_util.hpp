/**
 * @file math_util.hpp
 * @author WT
 * @brief 基础数学库
 * @note 一些常用的数学工具，包括判断素数、求最大公约数/最小公倍数、分解质因数、排列组合数等
 * @date 2019-07-26
 */
#pragma once

#include <cinttypes>
#include <cmath>
#include <map>
#include <vector>

namespace ytlib {

/**
 * @brief 计算有多少个1
 * @note 计算uint64_t的二进制形式中有多少个1
 * @param[in] n 输入的数字
 * @return uint8_t n的二进制形式中有多少个1
 */
inline uint8_t CountOne(uint64_t n) {
  uint8_t num = 0;
  while (n) {
    n &= (n - 1);
    ++num;
  }
  return num;
}

/**
 * @brief 计算有多少个0
 * @note 计算uint64_t的二进制形式中有多少个0
 * @param[in] n 输入的数字
 * @return uint8_t n的二进制形式中有多少个0
 */
inline uint8_t CountZero(uint64_t n) {
  return CountOne(~n);
}

/**
 * @brief 计算第一个1的位置
 * @note 计算uint64_t的二进制形式中在指定位置前第一个1的位置
 * @param[in] val 输入的数字
 * @param[in] pos 开始查找的位置
 * @return uint8_t 在指定位置前第一个1的位置。返回64表示没找到
 */
inline uint8_t FindFirstOne(uint64_t val, uint8_t pos = 0) {
  if (!val || pos >= 64) return 64;
  while (0 == (val & ((uint64_t)1 << pos)))
    ++pos;

  return pos;
}

/**
 * @brief 判断是否是质数
 * @note 利用【不在6的倍数两侧的一定不是质数】这条规律加快查找速度
 * @param[in] num 输入的数字
 * @return true 是质数
 * @return false 不是质数
 */
inline bool IsPrime(uint64_t num) {
  if (num < 2) return false;
  if (num == 2 || num == 3) return true;
  if (num % 6 != 1 && num % 6 != 5) return false;
  uint64_t tmp = uint64_t(std::sqrt(num));
  for (uint64_t i = 5; i <= tmp; i += 6)
    if (num % i == 0 || num % (i + 2) == 0)
      return false;
  return true;
}

/**
 * @brief 求最大公约数
 * 
 * @param[in] num1 输入的数字1
 * @param[in] num2 输入的数字2
 * @return uint64_t 最大公约数
 */
inline uint64_t Gcd(uint64_t num1, uint64_t num2) {
  if (num1 < num2) std::swap(num1, num2);
  if (num1 == 0) return 1;
  if (num2 == 0) return num1;
  if (num1 & 1) {
    if (num2 & 1) return Gcd(num2, num1 - num2);
    return Gcd(num1, num2 >> 1);
  } else {
    if (num2 & 1) return Gcd(num1 >> 1, num2);
    return (Gcd(num1 >> 1, num2 >> 1) << 1);
  }
}

/**
 * @brief 求最小公倍数
 * @note num1*num2/gcd(num1,num2)
 * @param[in] num1 输入的数字1
 * @param[in] num2 输入的数字2
 * @return uint64_t 最小公倍数
 */
inline uint64_t Lcm(uint64_t num1, uint64_t num2) {
  return num1 / Gcd(num1, num2) * num2;
}

/**
 * @brief 分解质因数
 * 
 * @param[in] num 待分解数
 * @return std::vector<uint64_t> 质因数列表
 */
inline std::vector<uint64_t> Factoring2Vec(uint64_t num) {
  std::vector<uint64_t> re;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      re.emplace_back(ii);
      num /= ii;
    }
  }
  return re;
}

/**
 * @brief 分解质因数
 * 
 * @param[in] num 待分解数
 * @return std::map<uint64_t, uint64_t> 质因数列表
 */
inline std::map<uint64_t, uint64_t> Factoring2Map(uint64_t num) {
  std::map<uint64_t, uint64_t> re;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      std::pair<std::map<uint64_t, uint64_t>::iterator, bool> tmp = re.emplace(ii, 1);
      if (!tmp.second) ++(tmp.first->second);
      num /= ii;
    }
  }
  return re;
}

/**
 * @brief 累乘
 * @note n(n-1)...m，需保证n>=m，且m!=0
 * @param[in] n 累乘开始数
 * @param[in] m 累乘结束数
 * @return uint64_t 累乘结果，有错误则返回0
 */
inline uint64_t Mul(uint64_t n, uint64_t m = 1) {
  if (n < m || m == 0) return 0;
  uint64_t re = n;
  while ((--n) >= m) re *= n;
  return re;
}

/**
 * @brief 排列数A(n,m)
 * @note 从n个不同元素中，任取m(m≤n,m与n均为自然数）个元素，其排列的个数。A(n,m)=n!/(n-m)!
 * @param[in] n A(n,m)中n
 * @param[in] m A(n,m)中m
 * @return uint64_t 排列数结果
 */
inline uint64_t Arn(uint64_t n, uint64_t m) {
  return Mul(n, n - m + 1);
}

/**
 * @brief 排列数A(n,m)递归求解
 * @note 已知A(n,mr)求A(n,m)
 * @param[in] n A(n,m)中n
 * @param[in] m A(n,m)中m
 * @param[in] a A(n,mr)值
 * @param[in] mr A(n,mr)中mr
 * @return uint64_t 排列数结果
 */
inline uint64_t ArnRec(uint64_t n, uint64_t m, uint64_t a, uint64_t mr) {
  if (n < m || m == 0 || n < mr || mr == 0) return 0;
  if (m < mr)
    return a / Mul(n - m, n - mr + 1);
  else if (m > mr)
    return a * Mul(n - mr, n - m + 1);
  else
    return a;
}

/**
 * @brief 组合数C(n,m)
 * @note 从n个不同元素中，任取m(m≤n,m与n均为自然数）个元素，其组合的个数。
 * C(n,m)=A(n,m)/m!=n!/(m!*(n-m)!)
 * C(n,m)=C(n,n-m)
 * @param[in] n C(n,m)中n
 * @param[in] m C(n,m)中m
 * @return uint64_t 组合数结果
 */
inline uint64_t Crn(uint64_t n, uint64_t m) {
  if (n < m || m == 0) return 0;
  if (n > (2 * m)) m = n - m;
  return (n == m) ? 1 : (Mul(n, m + 1) / Mul(n - m));
}

/**
 * @brief 组合数C(n,m)递归求解
 * @note 已知C(n,mr)求C(n,m)
 * @param[in] n C(n,m)中n
 * @param[in] m C(n,m)中m
 * @param[in] c C(n,mr)值
 * @param[in] mr C(n,mr)中mr
 * @return uint64_t 组合数结果
 */
inline uint64_t CrnRec(uint64_t n, uint64_t m, uint64_t c, uint64_t mr) {
  if (n < m || m == 0 || n < mr || mr == 0) return 0;
  if (n > (2 * m)) m = n - m;
  if (m < mr)
    return c * Mul(mr, m + 1) / Mul(n - m, n - mr + 1);
  else if (m > mr)
    return c * Mul(n - mr, n - m + 1) / Mul(m, mr + 1);
  else
    return c;
}

/**
 * @brief 等差数列求和
 * @param[in] a1 首项
 * @param[in] d 公差
 * @param[in] n 项数
 * @return double 等差数列的和
 */
inline double SumAP(double a1, double d, uint64_t n) {
  return n * a1 + ((n - 1) * n / 2) * d;
}

/**
 * @brief 等比数列求和
 * @param[in] a1 首项
 * @param[in] q 公比
 * @param[in] n 项数
 * @return double 等比数列的和
 */
inline double SumGP(double a1, double q, uint64_t n) {
  if (std::abs(q) < 1e-6) return 0.0;
  return (q == 1.0) ? (n * a1) : (a1 * (1 - std::pow(q, n)) / (1 - q));
}

/**
 * @brief 求整数次方数
 * @param[in] value 底数
 * @param[in] n 次方数
 * @return double 等比数列的和
 */
inline double Pow(double value, uint64_t n) {
  if (std::abs(value) < 1e-6) return 0.0;
  double re = 1.0;
  for (; n; n >>= 1) {
    if (n & 1)
      re *= value;
    value *= value;
  }
  return re;
}

}  // namespace ytlib