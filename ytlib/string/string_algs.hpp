/**
 * @file string_algs.hpp
 * @author WT
 * @brief 常用字符串算法
 * @note 常用字符串算法，包括kmp、差异度计算、最长不重复字串等。此处的算法都只是给出一种可行方案，不代表生产中的最佳方案
 * @date 2019-07-26
 */
#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace ytlib {

/**
 * @brief 字符串匹配kmp算法
 * @note 如果没有匹配到则返回sslen。实际使用中应根据情况先做好next字典
 * @param[in] ss 要被匹配的长字符串
 * @param[in] ps 要匹配的短字符串
 * @return size_t 字符串ps首次出现在ss中的位置，如果未出现则返回ss.length()
 */
inline size_t KMP(std::string_view ss, std::string_view ps) {
  size_t sslen = ss.length();
  size_t pslen = ps.length();

  if (pslen > sslen || pslen == 0) return sslen;
  std::vector<int32_t> next(pslen);
  next[0] = -1;
  int32_t ii = 0, jj = -1;
  --pslen;
  while (ii < pslen) {
    if (jj == -1 || ps[ii] == ps[jj]) {
      ++ii;
      ++jj;
      next[ii] = (ps[ii] == ps[jj]) ? next[jj] : jj;
    } else
      jj = next[jj];
  }
  ++pslen;
  ii = jj = 0;
  while (ii < sslen && jj < int32_t(pslen)) {
    if (jj == -1 || ss[ii] == ps[jj]) {
      ++ii;
      ++jj;
    } else
      jj = next[jj];
  }
  return (jj == pslen) ? (ii - jj) : sslen;
}

/**
 * @brief 计算字符串差异度
 * @note 优化方案只使用2*min(s1len,s2len)的内存而不是s1len*s2len的内存
 * @param[in] s1 字符串1
 * @param[in] s2 字符串2
 * @return size_t 字符串1和字符串2的差异度
 */
inline size_t StrDif(std::string_view s1, std::string_view s2) {
  //默认s1.length()>=s2.length()
  if (s2.length() > s1.length())
    return StrDif(s2, s1);

  // c1: unmatched cost; c2: mismatched cost
  const uint32_t c1 = 1, c2 = 1;

  size_t s1len = s1.length();
  size_t s2len = s2.length();
  std::vector<size_t> v1(s2len + 1), v2(s2len + 1);
  size_t *M1 = v1.data(), *M2 = v2.data(), *tmpM;

  for (size_t ii = 0; ii <= s2len; ++ii) {
    M1[ii] = ii * c1;
  }

  for (size_t ii = 1; ii <= s1len; ++ii) {
    for (size_t jj = 0; jj <= s2len; ++jj) {
      if (jj == 0)
        M2[0] = ii * c1;
      else {
        size_t val1 = ((s1[ii - 1] == s2[jj - 1]) ? 0 : c2) + M1[jj - 1];
        size_t val2 = std::min(M1[jj], M2[jj - 1]) + c1;
        M2[jj] = std::min(val1, val2);
      }
    }
    tmpM = M1;
    M1 = M2;
    M2 = tmpM;
  }

  return M1[s2len];
}

/**
 * @brief 最长不重复子串
 * @note 计算最先出现的最长的不含有重复字符的子串。返回其出现的位置和长度
 * @param[in] s 字符串
 * @return std::pair<size_t, size_t> 最长不重复子串出现的位置和长度
 */
inline std::pair<size_t, size_t> LongestSubStrWithoutDup(std::string_view s) {
  size_t len = s.length();
  size_t positions[256];                                    //每种字符上一次出现的位置
  for (size_t ii = 0; ii < len; ++ii) positions[ii] = len;  //初始化为len，表示没出现
  size_t maxLen = 0, maxPos = 0;                            //最长的字串长度和位置
  size_t curLen = 0, curPos = 0;                            //当前不重复字串的长度和位置
  for (size_t ii = 0; ii < len; ++ii) {
    size_t& prePos = positions[s[ii]];
    if (prePos == len || (ii - prePos) > curLen) {
      ++curLen;
    } else {
      if (curLen > maxLen) {
        maxLen = curLen;
        maxPos = curPos;
      }
      curLen = ii - prePos;
      curPos = prePos + 1;
    }
    prePos = ii;
  }

  if (curLen > maxLen) {
    maxLen = curLen;
    maxPos = curPos;
  }

  return std::pair<size_t, size_t>(maxPos, maxLen);
}

}  // namespace ytlib
