/**
 * @file string_algs.hpp
 * @brief 常用字符串算法
 * @details 常用字符串算法，包括kmp、差异度计算、最长不重复字串等。
 * 此处的算法都只是给出一种可行方案，不代表生产中的最佳方案
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <map>
#include <string>
#include <vector>

#include "string_tools_exports.h"

namespace ytlib {
/**
 * @brief 字符串匹配kmp算法
 * @details 如果没有匹配到则返回sslen。实际使用中应根据情况先做好next字典
 * @param ss 要被匹配的长字符串
 * @param sslen 字符串ss长度
 * @param ps 要匹配的短字符串
 * @param pslen 字符串ps的长度
 * @return 字符串ps首次出现在ss中的位置，如果未出现则返回sslen
 */
STRING_TOOLS_API std::size_t KMP(const char* ss, std::size_t sslen, const char* ps, std::size_t pslen);
/**
 * @brief 字符串匹配kmp算法（std::string形式）
 * @details 调用了char数组形式kmp
 * @param ss 要被匹配的长字符串
 * @param ps 要匹配的短字符串
 * @return 字符串ps首次出现在ss中的位置，如果未出现则返回ss.length()
 */
inline std::size_t KMP(const std::string& ss, const std::string& ps) {
  return KMP(ss.c_str(), ss.length(), ps.c_str(), ps.length());
}

/**
 * @brief 计算字符串差异度
 * @details 优化方案只使用2*min(s1len,s2len)的内存而不是s1len*s2len的内存
 * @param s1 字符串1
 * @param s1len 字符串1长度
 * @param s2 字符串2
 * @param s2len 字符串2长度
 * @return 字符串1和字符串2的差异度
 */
STRING_TOOLS_API std::size_t StrDif(const char* s1, std::size_t s1len, const char* s2, std::size_t s2len);
/**
 * @brief 计算字符串差异度（std::string形式）
 * @details 调用char数组形式StrDif
 * @param s1 字符串1
 * @param s2 字符串2
 * @return 字符串1和字符串2的差异度
 */
inline std::size_t StrDif(const std::string& s1, const std::string& s2) {
  return StrDif(s1.c_str(), s1.length(), s2.c_str(), s2.length());
}

/**
 * @brief 最长不重复子串
 * @details 计算最先出现的最长的不含有重复字符的子串。返回其出现的位置和长度
 * @param s 字符串
 * @param len 字符串长度
 * @return 最长不重复子串出现的位置和长度
 */
STRING_TOOLS_API std::pair<std::size_t, std::size_t> LongestSubStrWithoutDup(const char* s, std::size_t len);

inline std::pair<std::size_t, std::size_t> LongestSubStrWithoutDup(const std::string& s) {
  return LongestSubStrWithoutDup(s.c_str(), s.length());
}

}  // namespace ytlib
