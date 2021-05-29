/**
 * @file math_tools.cpp
 * @brief 基础数学库
 * @details 一些常用的数学工具，包括判断素数、求最大公约数/最小公倍数、分解质因数、排列组合数等
 * @author WT
 * @date 2019-07-26
 */
#include "math_tools.hpp"

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <map>
#include <vector>

//一些工具
namespace ytlib {

uint8_t CountOne(uint64_t n) {
  uint8_t num = 0;
  while (n) {
    n &= (n - 1);
    ++num;
  }
  return num;
}

uint8_t FindFirstOne(uint64_t val, uint8_t pos) {
  assert(pos < 64);
  if (!val) return 64;
  while (0 == (val & ((uint64_t)1 << pos)))
    ++pos;

  return pos;
}

bool IsPrime(const uint64_t& num) {
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

uint64_t Gcd(uint64_t num1, uint64_t num2) {
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

void Factoring(uint64_t num, std::vector<uint64_t>& re) {
  if (num < 2) return;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      re.emplace_back(ii);
      num /= ii;
    }
  }
}

void Factoring(uint64_t num, std::map<uint64_t, uint64_t>& re) {
  if (num < 2) return;
  for (uint64_t ii = 2; num > 1; ++ii) {
    while (num % ii == 0) {
      std::pair<std::map<uint64_t, uint64_t>::iterator, bool> tmp = re.emplace(ii, 1);
      if (!tmp.second) ++(tmp.first->second);
      num /= ii;
    }
  }
}

uint64_t Mul(uint64_t n, uint64_t m) {
  assert(n >= m && m);
  uint64_t re = n;
  while ((--n) >= m) re *= n;
  return re;
}

uint64_t Arn(uint64_t n, uint64_t m) {
  assert(n >= m && m);
  return Mul(n, n - m + 1);
}

uint64_t Arn(uint64_t n, uint64_t m, uint64_t a, uint64_t mr) {
  assert(n >= m && m && n >= mr && mr);
  if (m < mr)
    return a / Mul(n - m + 2, n - mr + 1);
  else if (m > mr)
    return a * Mul(n - mr + 2, n - m + 1);
  else
    return a;
}

uint64_t Crn(uint64_t n, uint64_t m) {
  assert(n >= m && m);
  if (n >= (2 * m + 1)) m = n - m;
  return (n == m) ? 1 : (Mul(n, m + 1) / Mul(n - m));
}

uint64_t Crn(uint64_t n, uint64_t m, uint64_t c, uint64_t mr) {
  assert(n >= m && m && n >= mr && mr);
  if (n >= (2 * m + 1)) m = n - m;
  if (m < mr)
    return c * Mul(mr, m + 1) / Mul(n - m, n - mr + 1);
  else if (m > mr)
    return c * Mul(n - mr, n - m + 1) / Mul(m, mr + 1);
  else
    return c;
}

}  // namespace ytlib