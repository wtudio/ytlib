#include "string_algs.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace ytlib {

std::size_t KMP(const char* ss, std::size_t sslen, const char* ps, std::size_t pslen) {
  assert(sslen && pslen && ss != NULL && ps != NULL);
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

std::size_t StrDif(const char* s1, std::size_t s1len, const char* s2, std::size_t s2len) {
  assert(s1len && s2len && s1 != NULL && s2 != NULL);
  if (s2len > s1len) return StrDif(s2, s2len, s1, s1len);  //默认s1len>=s2len

  const uint32_t c1 = 1, c2 = 1;  //c1: unmatched cost; c2: mismatched cost
  std::vector<std::size_t> v1(s2len), v2(s2len);
  std::size_t *M1 = v1.data(), *M2 = v2.data(), *tmpM;
  for (std::size_t ii = 0; ii < s2len; ++ii) M1[ii] = ii * c1;
  for (std::size_t ii = 1; ii < s1len; ++ii) {
    for (std::size_t jj = 0; jj < s2len; ++jj) {
      if (jj == 0)
        M2[0] = ii * c1;
      else {
        std::size_t val1 = ((s1[ii] == s2[jj]) ? 0 : c2) + M1[jj - 1];
        std::size_t val2 = std::min(M1[jj], M2[jj - 1]) + c1;
        M2[jj] = std::min(val1, val2);
      }
    }
    tmpM = M1;
    M1 = M2;
    M2 = tmpM;
  }
  std::size_t re = M1[s2len - 1];
  return re;
}

std::pair<std::size_t, std::size_t> LongestSubStrWithoutDup(const char* s, std::size_t len) {
  assert(len && s != NULL);
  std::size_t positions[256];                                    //每种字符上一次出现的位置
  for (std::size_t ii = 0; ii < len; ++ii) positions[ii] = len;  //初始化为len，表示没出现
  std::size_t maxLen = 0, maxPos = 0;                            //最长的字串长度和位置
  std::size_t curLen = 0, curPos = 0;                            //当前不重复字串的长度和位置
  for (std::size_t ii = 0; ii < len; ++ii) {
    std::size_t& prePos = positions[s[ii]];
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

  return std::pair<std::size_t, std::size_t>(maxPos, maxLen);
}

}  // namespace ytlib
